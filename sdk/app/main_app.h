/*
 * main_app.h
 *
 *  Created on: Nov 11, 2026
 *      Author: vanlo
 */

#ifndef SDK_APP_MAIN_APP_H_
#define SDK_APP_MAIN_APP_H_
#include "sm_hal.h"

#include "mpu_6050.h"
#include "attitude.h"
#include "sm_remote.h"
#include "motor_output.h"

#include "sm_storage.h"

typedef struct main_app {


	sm_attitude_t* m_attitude;
	sm_remote_t* m_remote;
	struct {

		mpu_6050_t* m_mpu;

		sm_hal_pwm_t* m_pwm_channel2;
		sm_motor_output_t* m_motor_out;
	}m_drive;

	struct {

		mpu6050_data_t m_mpu_config;
	}m_config;

	struct {

		sm_storage_config_t* m_mpu_storage;
	}m_storage;

	volatile bool m_motor_run;	/* toggle trong ngat UART, flightTask doc/xoa */
}main_app_t;

main_app_t* main_app_init(void);

extern main_app_t* main_app;
#endif /* SDK_APP_MAIN_APP_H_ */
