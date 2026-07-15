#include "alarm.h"

void Alarm_Init(AlarmManager_t *manager)
{
    manager->vibration_alarm_count = 0;
    manager->vibration_recover_count = 0;

    manager->state.vibration_alarm = 0;
    manager->state.temperature_alarm = 0;
    manager->state.humidity_alarm = 0;
    manager->state.system_alarm = 0;
}

void Alarm_Update(AlarmManager_t *manager,
                  int32_t vibration,
                  uint8_t temperature,
                  uint8_t humidity,
                  uint8_t dht_valid,
                  uint8_t mpu_valid,
                  uint8_t dht_fault,
                  uint8_t mpu_fault)
{
    /*
     * 一、振动报警状态机
     */
    if (mpu_fault == 1)
    {
        /*
         * MPU已经被确认故障。
         * 振动数据不可信，清除振动报警状态和计数。
         *
         * 系统仍然会因为mpu_fault而报警。
         */
        manager->state.vibration_alarm = 0;
        manager->vibration_alarm_count = 0;
        manager->vibration_recover_count = 0;
    }
    else if (mpu_valid == 0)
    {
        /*
         * 本轮读取失败，但还没有达到故障确认次数。
         *
         * 不使用上一次旧振动数据进行判断；
         * 保留当前振动报警状态；
         * 同时打断连续报警和连续恢复计数。
         */
        manager->vibration_alarm_count = 0;
        manager->vibration_recover_count = 0;
    }
    else if (manager->state.vibration_alarm == 0)
    {
        /*
         * MPU数据有效，并且当前未报警。
         * 统计连续超过报警阈值的次数。
         */
        manager->vibration_recover_count = 0;

        if (vibration > VIBRATION_ALARM_THRESHOLD)
        {
            if (manager->vibration_alarm_count < 255)
            {
                manager->vibration_alarm_count++;
            }
        }
        else
        {
            manager->vibration_alarm_count = 0;
        }

        if (manager->vibration_alarm_count >=
            VIBRATION_ALARM_COUNT_LIMIT)
        {
            manager->state.vibration_alarm = 1;
            manager->vibration_alarm_count = 0;
        }
    }
    else
    {
        /*
         * MPU数据有效，并且当前已经报警。
         * 统计连续低于恢复阈值的次数。
         */
        manager->vibration_alarm_count = 0;

        if (vibration < VIBRATION_RECOVER_THRESHOLD)
        {
            if (manager->vibration_recover_count < 255)
            {
                manager->vibration_recover_count++;
            }
        }
        else
        {
            manager->vibration_recover_count = 0;
        }

        if (manager->vibration_recover_count >=
            VIBRATION_RECOVER_COUNT_LIMIT)
        {
            manager->state.vibration_alarm = 0;
            manager->vibration_recover_count = 0;
        }
    }

    /*
     * 二、温湿度报警状态机
     */
    if (dht_fault == 1)
    {
        /*
         * DHT11已经确认故障，
         * 不继续使用旧温湿度数据。
         */
        manager->state.temperature_alarm = 0;
        manager->state.humidity_alarm = 0;
    }
    else if (dht_valid == 1)
    {
        /*
         * 温度滞回判断。
         */
        if (manager->state.temperature_alarm == 0)
        {
            if (temperature >= TEMP_HIGH_THRESHOLD)
            {
                manager->state.temperature_alarm = 1;
            }
        }
        else
        {
            if (temperature <= TEMP_RECOVER_THRESHOLD)
            {
                manager->state.temperature_alarm = 0;
            }
        }

        /*
         * 湿度滞回判断。
         */
        if (manager->state.humidity_alarm == 0)
        {
            if (humidity >= HUM_HIGH_THRESHOLD)
            {
                manager->state.humidity_alarm = 1;
            }
        }
        else
        {
            if (humidity <= HUM_RECOVER_THRESHOLD)
            {
                manager->state.humidity_alarm = 0;
            }
        }
    }

    /*
     * 三、系统总报警。
     */
    if ((manager->state.vibration_alarm == 1) ||
        (manager->state.temperature_alarm == 1) ||
        (manager->state.humidity_alarm == 1) ||
        (dht_fault == 1) ||
        (mpu_fault == 1))
    {
        manager->state.system_alarm = 1;
    }
    else
    {
        manager->state.system_alarm = 0;
    }
}