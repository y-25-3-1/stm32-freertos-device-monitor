#include "fault_monitor.h"

void FaultMonitor_Init(FaultMonitor_t *monitor)
{
    monitor->fault_count = 0;
    monitor->recover_count = 0;
    monitor->fault_flag = 0;
}

void FaultMonitor_Update(FaultMonitor_t *monitor,
                         uint8_t read_ok,
                         uint8_t fault_limit,
                         uint8_t recover_limit)
{
    /*
     * 本次读取成功。
     */
    if (read_ok == 1)
    {
        /*
         * 连续失败被打断。
         */
        monitor->fault_count = 0;

        /*
         * 如果此前已经确认故障，
         * 开始统计连续恢复次数。
         */
        if (monitor->fault_flag == 1)
        {
            if (monitor->recover_count < 255)
            {
                monitor->recover_count++;
            }

            if (monitor->recover_count >= recover_limit)
            {
                monitor->fault_flag = 0;
                monitor->recover_count = 0;
            }
        }
        else
        {
            /*
             * 本来就没有故障，
             * 不需要统计恢复次数。
             */
            monitor->recover_count = 0;
        }
    }
    else
    {
        /*
         * 本次读取失败，
         * 连续恢复被打断。
         */
        monitor->recover_count = 0;

        /*
         * 如果当前还没有确认故障，
         * 继续统计连续失败次数。
         */
        if (monitor->fault_flag == 0)
        {
            if (monitor->fault_count < 255)
            {
                monitor->fault_count++;
            }

            if (monitor->fault_count >= fault_limit)
            {
                monitor->fault_flag = 1;
                monitor->fault_count = 0;
            }
        }
        else
        {
            /*
             * 已经处于故障状态，
             * 不继续累计失败次数。
             */
            monitor->fault_count = 0;
        }
    }
}