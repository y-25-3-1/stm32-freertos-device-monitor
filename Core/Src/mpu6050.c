#include "mpu6050.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    if (HAL_I2C_Mem_Write(&hi2c1,
                          MPU6050_ADDR,
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100) == HAL_OK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *data)
{
    if (HAL_I2C_Mem_Read(&hi2c1,
                         MPU6050_ADDR,
                         reg,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         1,
                         100) == HAL_OK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static uint8_t MPU6050_ReadRegs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (HAL_I2C_Mem_Read(&hi2c1,
                         MPU6050_ADDR,
                         reg,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         len,
                         100) == HAL_OK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t MPU6050_ReadID(void)
{
    uint8_t id = 0;

    MPU6050_ReadReg(MPU6050_REG_WHO_AM_I, &id);

    return id;
}

uint8_t MPU6050_Init(void)
{
    uint8_t id;

    HAL_Delay(100);

    id = MPU6050_ReadID();

    if ((id != 0x68) && (id != 0x70))
		{
				return 1;
		}

    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x00);
    HAL_Delay(10);

    MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x07);
    MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x06);
    MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x00);
    MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x00);

    return 0;
}

uint8_t MPU6050_ReadAccel(MPU6050_Data_t *data)
{
    uint8_t buf[6];

    if (MPU6050_ReadRegs(MPU6050_REG_ACCEL_XOUT_H, buf, 6) != 0)
    {
        return 1;
    }

    data->ax = (int16_t)((buf[0] << 8) | buf[1]);
    data->ay = (int16_t)((buf[2] << 8) | buf[3]);
    data->az = (int16_t)((buf[4] << 8) | buf[5]);

    return 0;
}
