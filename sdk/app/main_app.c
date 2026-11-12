/*
 * main_app.c
 *
 *  Created on: Nov 11, 2026
 *      Author: vanlo
 */
#include "i2c.h"

#include "main_app.h"

static main_app_t g_main_app = {

		.m_mpu = NULL,
		.m_drive = {
				.mpu_i2c = NULL,
		}
};

main_app_t* main_app = NULL;

static int32_t main_app_drive_init(main_app_t* _this){

	sm_hal_i2c_t*i2c =  sm_hal_i2c_init(&hi2c1, 0);
	if(!i2c || !_this) return -1;
	_this->m_drive.mpu_i2c = i2c;
	sm_hal_i2c_open(i2c);
	return 0;
}

static int32_t main_app_mpu_6050_init(main_app_t* _this){

	mpu_6050_t*mpu = mpu_6050_init(_this->m_drive.mpu_i2c);
	if(!mpu || !_this) return -1;
	_this->m_mpu = mpu;
	mpu_6050_set_power_ctrl(mpu, 1);
	mpu_6050_set_gyro_config(mpu,GYRO_CONFIG_250_DEG_S);
	mpu_6050_set_accel_config(mpu,ACCEL_CONFIG_2_G);
	mpu_6050_calibrate(mpu) ;
	return 0;
}

main_app_t* main_app_init(void){

	main_app = &g_main_app;
	if(main_app_drive_init(main_app) <  0) {
		return NULL;
	}
	if(main_app_mpu_6050_init(main_app) < 0){
		return NULL;
	}
	return main_app;
}
