/*
 * angle_controll.h
 *
 *  Created on: Nov 18, 2026
 *      Author: vanlo
 */

#ifndef SDK_APP_ANGLE_CONTROLL_H_
#define SDK_APP_ANGLE_CONTROLL_H_

#include <stdint.h>
#include "attitude.h"

typedef void sm_motor_control_t;


typedef struct {
    float roll_set;      // deg
    float pitch_set;     // deg
    float yaw_rate_set;  // deg/s
} sm_motor_control_sp_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
} sm_motor_control_output_t;

sm_motor_control_t* sm_motor_control_init(void);

void sm_motor_control_reset(sm_motor_control_t *_this);

void sm_motor_control_deinit(sm_motor_control_t *_this);

int8_t sm_motor_control_update(sm_motor_control_t *_this,const sm_attitude_angle_t *attitude,
		const sm_motor_control_sp_t *sp, float dt, sm_motor_control_output_t *ref);

#endif /* SDK_APP_ANGLE_CONTROLL_H_ */
