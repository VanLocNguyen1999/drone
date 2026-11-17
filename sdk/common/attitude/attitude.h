/*
 * attitude.h
 *
 *  Created on: Nov 12, 2026
 *      Author: vanlo
 */

#ifndef SDK_COMMON_ATTITUDE_ATTITUDE_H_
#define SDK_COMMON_ATTITUDE_ATTITUDE_H_
#include "mpu_6050.h"

typedef struct {
	float roll;
	float pitch;
	float yaw;
} sm_attitude_angle_t;

typedef struct {

	mpu_6050_t *m_mpu;
	sm_attitude_angle_t angle;

	float q0, q1, q2, q3;
	float integral_x, integral_y, integral_z;
	float kp, ki;

	uint32_t last_tick;
	uint8_t started;

	bool m_is_calib;
} sm_attitude_t;

sm_attitude_t* sm_attitude_est_init(mpu_6050_t *mpu);
int32_t sm_attitude_est_set_gain(sm_attitude_t *est, float kp, float ki);
int32_t sm_attitude_est_update(sm_attitude_t *est);
int32_t sm_attitude_est_get_angle(const sm_attitude_t *est,sm_attitude_angle_t *angle);

#endif /* SDK_COMMON_ATTITUDE_ATTITUDE_H_ */
