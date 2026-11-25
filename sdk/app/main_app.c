/*
 * main_app.c
 *
 *  Created on: Nov 11, 2026
 *      Author: vanlo
 */
#include "i2c.h"

#include "main_app.h"

#include "sm_flash_config.h"
#include "app_bsp.h"

static main_app_t g_main_app = {

		.m_attitude = NULL,
		.m_drive = {
				.m_mpu = NULL,
		},
		.m_storage = {
				.m_mpu_storage = NULL,
		},
		.m_motor_run = 0,
};

main_app_t* main_app = NULL;
static int32_t main_app_storage_init(main_app_t* _this){
	if( !_this) return -1;
	sm_storage_config_t* storage = NULL;
	sm_hal_flash_t* flash = sm_bsp_get_flash();
	storage = sm_storage_create(flash, FLASH_MPU_OFFSET_CONFIG, sizeof(mpu6050_data_t));
	if(!storage) return -1;
	_this->m_storage.m_mpu_storage = storage;
	return 0;
}
static int32_t main_app_load_config(main_app_t* _this){
	if( !_this) return -1;
	if(_this->m_storage.m_mpu_storage){
		if (_this->m_storage.m_mpu_storage->m_base.m_proc->load(&_this->m_storage.m_mpu_storage->m_base,
				&_this->m_config.m_mpu_config) < 0) {
			if (mpu_6050_calibrate(_this->m_drive.m_mpu, &_this->m_config.m_mpu_config) >= 0){

				_this->m_storage.m_mpu_storage->m_base.m_proc->store(&_this->m_storage.m_mpu_storage->m_base,
								&_this->m_config.m_mpu_config);
			}
		}
		mpu_6050_set_offset(_this->m_drive.m_mpu, _this->m_config.m_mpu_config);
	}
	return 0;
}
static int32_t main_app_drive_init(main_app_t* _this){

	sm_hal_i2c_t *i2c = sm_bsp_get_mpu_i2c();
	mpu_6050_t*mpu = mpu_6050_init(i2c);
	if(!mpu || !_this) return -1;
	_this->m_drive.m_mpu = mpu;
	mpu_6050_set_power_ctrl(mpu, 1);
	mpu_6050_set_gyro_config(mpu,GYRO_CONFIG_1000_DEG_S);
	mpu_6050_set_accel_config(mpu,ACCEL_CONFIG_8_G);

	_this->m_drive.m_pwm_channel2 = sm_bsp_get_pwm2();
	return 0;
}

static int32_t main_app_attitude_init(main_app_t* _this){

	sm_attitude_t*attitude = sm_attitude_est_init(_this->m_drive.m_mpu);
	if(!attitude || !_this) return -1;
	_this->m_attitude  = attitude;
	return 0;
}

static uint8_t last_sw = 0;
static void sm_remote_frame_callback(const sm_remote_ctrl_t *ctrl, void *arg){


	if(!ctrl || !arg) return;
	main_app_t* _this = (main_app_t*) arg;
	if(ctrl->sw && !last_sw) {

		_this->m_motor_run = !_this->m_motor_run;
		if(_this->m_motor_run){

			sm_hal_pwm_set_duty_sat(_this->m_drive.m_pwm_channel2, 0.95f, 0.3f);
		}else {
			sm_hal_pwm_set_duty_sat(_this->m_drive.m_pwm_channel2, 0.0f, 0.0f);
		}
	}
	last_sw = ctrl->sw;
}

static int32_t main_app_remote_init(main_app_t* _this){

	sm_hal_uart_t* uart_remote = sm_bsp_get_uart1();
	sm_remote_t*remote = sm_remote_init(uart_remote,1000);
	if(!remote || !_this) return -1;
	_this->m_remote  = remote;
	sm_remote_set_frame_callback(remote, sm_remote_frame_callback, _this);
	sm_remote_start(remote);
	return 0;
}

main_app_t* main_app_init(void){

	main_app = &g_main_app;
	if(main_app_drive_init(main_app) < 0){
		return NULL;
	}
	main_app_storage_init(main_app);

	if(main_app_load_config(main_app) < 0){
		return NULL;
	}
	if(main_app_attitude_init(main_app) < 0){
		return NULL;
	}
	if(main_app_remote_init(main_app) < 0){
		return NULL;
	}
	return main_app;
}
