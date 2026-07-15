#ifndef __ALARM_H
#define __ALARM_H

#include "main.h"

/* 振动报警参数 */
#define VIBRATION_ALARM_THRESHOLD       3000
#define VIBRATION_RECOVER_THRESHOLD     1800
#define VIBRATION_ALARM_COUNT_LIMIT     3
#define VIBRATION_RECOVER_COUNT_LIMIT   5

/* 温度报警参数 */
#define TEMP_HIGH_THRESHOLD             26
#define TEMP_RECOVER_THRESHOLD          24

/* 湿度报警参数 */
#define HUM_HIGH_THRESHOLD              85
#define HUM_RECOVER_THRESHOLD           80

/*
 * 保存最终报警状态。
 */
typedef struct
{
    uint8_t vibration_alarm;
    uint8_t temperature_alarm;
    uint8_t humidity_alarm;
    uint8_t system_alarm;
} AlarmState_t;

/*
 * 报警管理器。
 *
 * 除了保存最终状态，还保存连续报警和连续恢复次数。
 */
typedef struct
{
    uint8_t vibration_alarm_count;
    uint8_t vibration_recover_count;

    AlarmState_t state;
} AlarmManager_t;

/* 初始化报警管理器 */
void Alarm_Init(AlarmManager_t *manager);

/* 根据当前传感器数据更新报警状态 */
void Alarm_Update(AlarmManager_t *manager,
                  int32_t vibration,
                  uint8_t temperature,
                  uint8_t humidity,
                  uint8_t dht_valid,
                  uint8_t mpu_valid,
                  uint8_t dht_fault,
                  uint8_t mpu_fault);

#endif