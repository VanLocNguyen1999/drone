/*
 * main_app.h
 *
 *  Created on: Nov 11, 2026
 *      Author: vanlo
 */

#ifndef SDK_APP_MAIN_APP_H_
#define SDK_APP_MAIN_APP_H_
#include "sm_hal_i2c.h"

#include "mpu_6050.h"
#include "attitude.h"

#include "sm_storage.h"

typedef struct main_app {


	sm_attitude_t* m_attitude;

	struct {

		mpu_6050_t* m_mpu;;
	}m_drive;

	struct {

		mpu6050_data_t m_mpu_config;
	}m_config;

	struct {

		sm_storage_config_t* m_mpu_storage;
	}m_storage;
}main_app_t;

main_app_t* main_app_init(void);

extern main_app_t* main_app;
#endif /* SDK_APP_MAIN_APP_H_ */
