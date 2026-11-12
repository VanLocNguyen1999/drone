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

typedef struct main_app {

	mpu_6050_t* m_mpu;
	struct {

		sm_hal_i2c_t* mpu_i2c;
	}m_drive;
}main_app_t;

main_app_t* main_app_init(void);

extern main_app_t* main_app;
#endif /* SDK_APP_MAIN_APP_H_ */
