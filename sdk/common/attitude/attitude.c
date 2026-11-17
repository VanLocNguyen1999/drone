/*
 * attitude.c
 *
 *  Created on: Nov 12, 2026
 *      Author: vanlo
 */
#include "attitude.h"
#include "stdlib.h"

#define DEG_TO_RAD     0.01745329252f
#define RAD_TO_DEG     57.29577951f

#define INTEGRAL_LIMIT 0.1f

sm_attitude_t* sm_attitude_est_init(mpu_6050_t *mpu) {

	if (!mpu)return NULL;
	sm_attitude_t *this = (sm_attitude_t*) malloc(sizeof(sm_attitude_t));

	if (!this)return NULL;

	memset(this, 0, sizeof(sm_attitude_t));
	this->m_mpu = mpu;
	this->q0 = 1.0f;
	this->q1 = 0.0f;
	this->q2 = 0.0f;
	this->q3 = 0.0f;

	this->kp = 1.0f;
	this->ki = 0.1f;
	return this;
}

int32_t sm_attitude_est_set_gain(sm_attitude_t *this, float kp, float ki) {

	if (!this || !isfinite(kp) || !isfinite(ki) || kp < 0.0f || ki < 0.0f)
		return -1;
	this->kp = kp;
	this->ki = ki;
	if (ki == 0.0f) {
		this->integral_x = 0.0f;
		this->integral_y = 0.0f;
		this->integral_z = 0.0f;
	}
	return 0;
}

static void sm_attitude_est_correct_gyro(sm_attitude_t *this, const mpu6050_data_t *data, float gyro[3], float dt)
{
	float ax = data->accel_x;
	float ay = data->accel_y;
	float az = data->accel_z;

	float norm = sqrtf(ax * ax + ay * ay + az * az);
	if (norm > 0.8f && norm < 1.2f) {
		ax /= norm;
		ay /= norm;
		az /= norm;

		float q0 = this->q0;
		float q1 = this->q1;
		float q2 = this->q2;
		float q3 = this->q3;

		// Estimated gravity
		float vx = 2.0f * (q1 * q3 - q0 * q2);
		float vy = 2.0f * (q0 * q1 + q2 * q3);
		float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

		// Cross-product error
		float ex = ay * vz - az * vy;
		float ey = az * vx - ax * vz;
		float ez = ax * vy - ay * vx;

		if (this->ki > 0.0f) {
			this->integral_x += this->ki * ex * dt;
			this->integral_y += this->ki * ey * dt;
			this->integral_z += this->ki * ez * dt;

			this->integral_x = fmaxf(-INTEGRAL_LIMIT,fminf(INTEGRAL_LIMIT, this->integral_x));
			this->integral_y = fmaxf(-INTEGRAL_LIMIT,fminf(INTEGRAL_LIMIT, this->integral_y));
			this->integral_z = fmaxf(-INTEGRAL_LIMIT,fminf(INTEGRAL_LIMIT, this->integral_z));
		}

		gyro[0] += this->kp * ex;
		gyro[1] += this->kp * ey;
		gyro[2] += this->kp * ez;
	}

	gyro[0] += this->integral_x;
	gyro[1] += this->integral_y;
	gyro[2] += this->integral_z;
}
static int32_t sm_attitude_est_update_quaternion(sm_attitude_t *this,const float gyro[3], float dt) {

	float q0 = this->q0;
	float q1 = this->q1;
	float q2 = this->q2;
	float q3 = this->q3;

	float gx = gyro[0];
	float gy = gyro[1];
	float gz = gyro[2];

	float half_dt = 0.5f * dt;

	float nq0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * half_dt;
	float nq1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * half_dt;
	float nq2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * half_dt;
	float nq3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * half_dt;

	float norm = sqrtf(nq0 * nq0 + nq1 * nq1 + nq2 * nq2 + nq3 * nq3);

	if (!isfinite(norm) || norm < 1e-6f)
		return -1;

	this->q0 = nq0 / norm;
	this->q1 = nq1 / norm;
	this->q2 = nq2 / norm;
	this->q3 = nq3 / norm;

	return 0;
}
static void sm_attitude_est_update_angle(sm_attitude_t *this) {

	float q0 = this->q0;
	float q1 = this->q1;
	float q2 = this->q2;
	float q3 = this->q3;
	this->angle.roll = atan2f(2.0f * (q0 * q1 + q2 * q3),1.0f - 2.0f * (q1 * q1 + q2 * q2)) * RAD_TO_DEG;

	float sin_pitch = 2.0f * (q0 * q2 - q3 * q1);
	sin_pitch = fmaxf(-1.0f, fminf(1.0f, sin_pitch));

	this->angle.pitch = asinf(sin_pitch) * RAD_TO_DEG;
	this->angle.yaw = atan2f(2.0f * (q0 * q3 + q1 * q2),1.0f - 2.0f * (q2 * q2 + q3 * q3)) * RAD_TO_DEG;
}
int32_t sm_attitude_est_update(sm_attitude_t *this) {

	if (!this || !this->m_mpu)
		return -1;
	mpu6050_data_t data;
	if (mpu_6050_read_data(this->m_mpu, &data) < 0)
		return -1;

	uint32_t now = HAL_GetTick();
	if (!this->started) {
		this->started = 1;
		this->last_tick = now;
		return 0;
	}

	float dt = (now - this->last_tick) * 0.001f;
	if (dt <= 0.0f)
		return 0;

	this->last_tick = now;

	if (dt > 0.05f) return -1;

	float gyro[3] = { data.gyro_x * DEG_TO_RAD, data.gyro_y * DEG_TO_RAD, data.gyro_z * DEG_TO_RAD };

	if (!isfinite(data.accel_x) ||!isfinite(data.accel_y) ||!isfinite(data.accel_z)
			||!isfinite(gyro[0]) ||!isfinite(gyro[1]) || !isfinite(gyro[2]))
		return -1;

	sm_attitude_est_correct_gyro(this, &data, gyro, dt);
	if (sm_attitude_est_update_quaternion(this, gyro, dt) < 0)
		return -1;
	sm_attitude_est_update_angle(this);
	return 0;
}
int32_t sm_attitude_est_get_angle(const sm_attitude_t *est,sm_attitude_angle_t *angle) {

	if (!est || !angle)
		return -1;
	*angle = est->angle;
	return 0;
}
