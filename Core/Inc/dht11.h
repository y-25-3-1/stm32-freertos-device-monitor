#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

/* DHT11 使用的GPIO */
#define DHT11_GPIO_PORT    GPIOB
#define DHT11_GPIO_PIN     GPIO_PIN_11

/* 函数返回值 */
#define DHT11_OK                 0
#define DHT11_ERROR_TIMEOUT      1
#define DHT11_ERROR_CHECKSUM     2

/* 保存DHT11读取结果 */
typedef struct
{
    uint8_t temperature;
    uint8_t humidity;
} DHT11_Data_t;

/* 初始化DHT11驱动 */
void DHT11_Init(void);

/* 读取温湿度 */
uint8_t DHT11_Read(DHT11_Data_t *data);

#endif