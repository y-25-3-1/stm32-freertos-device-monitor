#ifndef __FAULT_MONITOR_H
#define __FAULT_MONITOR_H

#include "main.h"

/*
 * 通用传感器故障监视器。
 */
typedef struct
{
    uint8_t fault_count;
    uint8_t recover_count;
    uint8_t fault_flag;
} FaultMonitor_t;

/*
 * 初始化故障监视器。
 */
void FaultMonitor_Init(FaultMonitor_t *monitor);

/*
 * 根据本次读取结果更新故障状态。
 *
 * read_ok：
 * 1表示本次读取成功；
 * 0表示本次读取失败。
 *
 * fault_limit：
 * 连续失败多少次后确认故障。
 *
 * recover_limit：
 * 故障后连续成功多少次解除故障。
 */
void FaultMonitor_Update(FaultMonitor_t *monitor,
                         uint8_t read_ok,
                         uint8_t fault_limit,
                         uint8_t recover_limit);

#endif