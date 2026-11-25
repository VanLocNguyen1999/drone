/*
 * angle_control.c
 *
 *  Created on: Nov 18, 2026
 *      Author: vanlo
 */

#include "motor_control.h"
#include "pid_control.h"

#include <stdlib.h>
#include <math.h>

#define _impl(x) ((sm_motor_control_impl_t*)(x))

#define ANGLE_DT       0.01f
#define RATE_DT        0.002f
#define ANGLE_DIVIDER  5U

#define ANGLE_KP       4.0f
#define MAX_RATE       150.0f

#define ROLL_LIMIT     0.3f
#define PITCH_LIMIT    0.3f
#define YAW_LIMIT      0.2f

/* Rate PID: output chuan hoa cho mixer, input deg/s.
 * Gia tri khoi dau, PHAI tune tren gia (thao canh khi test dau). */
#define RATE_RP_KP     0.0015f
#define RATE_RP_KI     0.0030f
#define RATE_RP_KD     0.00003f
#define RATE_YAW_KP    0.0030f
#define RATE_YAW_KI    0.0050f
#define RATE_YAW_KD    0.0f

typedef struct {
	pid_control_t *angle;
	pid_control_t *rate;
} sm_axis_control_t;

typedef struct {
	sm_axis_control_t roll;
	sm_axis_control_t pitch;

	pid_control_t *yaw_rate;

	float roll_rate_sp;
	float pitch_rate_sp;

	uint8_t angle_divider;
} sm_motor_control_impl_t;

static sm_motor_control_impl_t g_motor_control;

static void free_pids(sm_motor_control_impl_t *this) {
	if(!this) return;
	sm_pid_control_deint(this->roll.angle);
	sm_pid_control_deint(this->roll.rate);
	sm_pid_control_deint(this->pitch.angle);
	sm_pid_control_deint(this->pitch.rate);
	sm_pid_control_deint(this->yaw_rate);
}

sm_motor_control_t* sm_motor_control_init(void) {
	sm_motor_control_impl_t *this = &g_motor_control;
	if (!this)
		return NULL;

	// Outer loop: Angle P, 100 Hz
	this->roll.angle = sm_pid_control_init(0, 0, ANGLE_KP, 0, ANGLE_DT);
	this->pitch.angle = sm_pid_control_init(0, 0, ANGLE_KP, 0, ANGLE_DT);

	// Inner loop: Rate PID, 500 Hz
	// PID gains must be tuned on test rig
	/* sm_pid_control_init(initial, ki, kp, kc, td) */
	this->roll.rate = sm_pid_control_init(0, RATE_RP_KI, RATE_RP_KP, 0, RATE_DT);
	this->pitch.rate = sm_pid_control_init(0, RATE_RP_KI, RATE_RP_KP, 0, RATE_DT);
	this->yaw_rate = sm_pid_control_init(0, RATE_YAW_KI, RATE_YAW_KP, 0, RATE_DT);

	if (!this->roll.angle || !this->pitch.angle || !this->roll.rate
			|| !this->pitch.rate || !this->yaw_rate) {

		free_pids(this);
		return NULL;
	}

	// Angle output = desired angular rate
	sm_pid_control_config(this->roll.angle, 0, MAX_RATE, 0, 0);
	sm_pid_control_config(this->pitch.angle, 0, MAX_RATE, 0, 0);

	// Rate outputs = normalized mixer commands
	sm_pid_control_config(this->roll.rate, RATE_RP_KD, ROLL_LIMIT, 0.2f, 30);
	sm_pid_control_config(this->pitch.rate, RATE_RP_KD, PITCH_LIMIT, 0.2f, 30);
	sm_pid_control_config(this->yaw_rate, RATE_YAW_KD, YAW_LIMIT, 0.1f, 30);

	return this;
}

void sm_motor_control_reset(sm_motor_control_t *_this) {
	if (!_this)
		return;

	sm_motor_control_impl_t *this = _impl(_this);

	sm_pid_control_reset(this->roll.angle);
	sm_pid_control_reset(this->roll.rate);
	sm_pid_control_reset(this->pitch.angle);
	sm_pid_control_reset(this->pitch.rate);
	sm_pid_control_reset(this->yaw_rate);

	this->roll_rate_sp = 0.0f;
	this->pitch_rate_sp = 0.0f;
	this->angle_divider = 0;
}

void sm_motor_control_deinit(sm_motor_control_t *_this) {
	if (!_this)
		return;

	sm_motor_control_impl_t *this = _impl(_this);
	free_pids(this);
	free(this);
}

int8_t sm_motor_control_update(sm_motor_control_t *_this,const sm_attitude_angle_t *attitude,
		const sm_motor_control_sp_t *sp, float dt, sm_motor_control_output_t *ref){
	if (!_this || !attitude || !sp || !ref)
		return -1;
	// Reject abnormal scheduler timing.
	if (!isfinite(dt) || dt < 0.0015f || dt > 0.0025f)
		return -2;

	if (!isfinite(attitude->roll) ||!isfinite(attitude->pitch)
			|| !isfinite(attitude->gx) ||!isfinite(attitude->gy) ||!isfinite(attitude->gz)
			|| !isfinite(sp->roll_set) ||!isfinite(sp->pitch_set) ||!isfinite(sp->yaw_rate_set))
		return -3;

	sm_motor_control_impl_t *this = _impl(_this);
	// Angle loop: 100 Hz
	if (this->angle_divider == 0) {

		this->roll_rate_sp = sm_pi_clamping_running(this->roll.angle,
				sp->roll_set - attitude->roll, 0, 0);
		this->pitch_rate_sp = sm_pi_clamping_running(this->pitch.angle,
				sp->pitch_set - attitude->pitch, 0, 0);
	}

	this->angle_divider++;

	if (this->angle_divider >= ANGLE_DIVIDER)
		this->angle_divider = 0;

	// Rate loop: 500 Hz
	ref->roll = sm_pid_running(this->roll.rate, this->roll_rate_sp,
			attitude->gx);
	ref->pitch = sm_pid_running(this->pitch.rate, this->pitch_rate_sp,
			attitude->gy);

	// Yaw: Rate PID only
	ref->yaw = sm_pid_running(this->yaw_rate, sp->yaw_rate_set,attitude->gz);

	ref->roll = clampf(ref->roll, -ROLL_LIMIT, ROLL_LIMIT);
	ref->pitch = clampf(ref->pitch, -PITCH_LIMIT, PITCH_LIMIT);
	ref->yaw = clampf(ref->yaw, -YAW_LIMIT, YAW_LIMIT);

	return 0;
}
