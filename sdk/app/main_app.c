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

/* ESC: 400 Hz (chu ky 2500 us), 1000..2000 us. O 500 Hz chu ky chi 2000 us
 * nen xung 2000 us = duty 100%, ESC khong doc duoc -> init se bao loi. */
/* motor_mix M1 FL, M2 FR, M3 RR, M4 RL
 * -> TIM2 CH1 PA5, CH2 PB3, CH3 PA2, CH4 PA3. Doi bang nay neu day noi khac. */
static const sm_motor_output_cfg_t g_motor_out_cfg = {
		.pwm = NULL,			/* gan trong main_app_drive_init */
		.channel = { PWM_CHANNEL_1, PWM_CHANNEL_2, PWM_CHANNEL_3, PWM_CHANNEL_4 },
		.freq_hz = 400,
		.us_min = 1000.0f,
		.us_max = 2000.0f,
		.mode = MOTOR_OUTPUT_MODE_ESC_US,
};

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

	sm_motor_output_cfg_t out_cfg = g_motor_out_cfg;
	out_cfg.pwm = _this->m_drive.m_pwm_channel2;
	_this->m_drive.m_motor_out = sm_motor_output_init(&out_cfg);
	if(!_this->m_drive.m_motor_out) return -1;
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
	/* Chi toggle co arm. Xuat PWM do flightTask lam (500 Hz). */
	if(ctrl->sw && !last_sw) {

		_this->m_motor_run = !_this->m_motor_run;
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
