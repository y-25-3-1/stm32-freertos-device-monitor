#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"

#define MPU6050_ADDR        0xD0

#define MPU6050_REG_SMPLRT_DIV    0x19
#define MPU6050_REG_CONFIG        0x1A
#define MPU6050_REG_GYRO_CONFIG   0x1B
#define MPU6050_REG_ACCEL_CONFIG  0x1C
#define MPU6050_REG_ACCEL_XOUT_H  0x3B
#define MPU6050_REG_WHO_AM_I      0x75
#define MPU6050_REG_PWR_MGMT_1    0x6B

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
} MPU6050_Data_t;

uint8_t MPU6050_Init(void);
uint8_t MPU6050_ReadID(void);
uint8_t MPU6050_ReadAccel(MPU6050_Data_t *data);

#endif
