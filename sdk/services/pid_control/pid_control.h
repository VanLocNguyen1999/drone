/*
 * sm_pid.h
 *
 *  Created on: Nov 18, 2026
 *      Author: vanlo
 */

#ifndef SDK_SERVICES_PID_CONTROL_SM_PID_H_
#define SDK_SERVICES_PID_CONTROL_SM_PID_H_
#include "sm_types.h"


typedef void pid_control_t;

pid_control_t* sm_pid_control_init(const float_t initial_value, const float_t ki,
		const float_t kp, const float_t kc, const float_t td);
void sm_pid_control_reset(pid_control_t *_this);
void sm_pid_control_deint(pid_control_t *_this);
void sm_pid_control_para(pid_control_t *_this, const float_t ki, const float_t kp,float kc, const float_t td);
void sm_pid_control_config(pid_control_t *_this, float_t kd, float_t output_limit,float_t integral_limit, float_t d_cutoff_hz);
float_t sm_pi_running(pid_control_t *_this, float_t f_error, float sat_input,float sat_output);
float_t sm_pi_clamping_running(pid_control_t *_this, float f_error,float sat_input, float sat_output);
float_t sm_pid_running(pid_control_t *_this, float_t _sp,float_t _fb);

#endif /* SDK_SERVICES_PID_CONTROL_SM_PID_H_ */
