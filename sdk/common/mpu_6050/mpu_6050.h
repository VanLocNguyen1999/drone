/*
 * mpu_6050.h
 *
 *  Created on: Nov 10, 2026
 *      Author: vanlo
 */

#ifndef SDK_COMMON_MPU_6050_MPU_6050_H_
#define SDK_COMMON_MPU_6050_MPU_6050_H_
#include "sm_hal_i2c.h"


typedef enum {

	GYRO_CONFIG_250_DEG_S = 0x00,
	GYRO_CONFIG_500_DEG_S = 0x08,
	GYRO_CONFIG_1000_DEG_S = 0x10,
	GYRO_CONFIG_2000_DEG_S = 0x18,
}GYRO_CONFIG_DATA;

typedef enum {

	ACCEL_CONFIG_2_G = 0x00,
	ACCEL_CONFIG_4_G = 0x08,
	ACCEL_CONFIG_8_G = 0x10,
	ACCEL_CONFIG_16_G = 0x18,
}ACCEL_CONFIG_DATA;

typedef void mpu_6050_t;

mpu_6050_t* mpu_6050_init(sm_hal_i2c_t* i2c);
int32_t mpu_6050_set_power_ctrl(mpu_6050_t *_this, uint8_t enable);
int32_t mpu_6050_set_gyro_config(mpu_6050_t* _this,GYRO_CONFIG_DATA gyro_config);
int32_t mpu_6050_set_accel_config(mpu_6050_t* _this,ACCEL_CONFIG_DATA accel_config);
int32_t mpu_6050_calibrate(mpu_6050_t* _this) ;
void mpu_6050_process(mpu_6050_t* _this, uint32_t time_diff);
#endif /* SDK_COMMON_MPU_6050_MPU_6050_H_ */
