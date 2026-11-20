/*
 * sm_pid.c
 *
 *  Created on: Nov 18, 2026
 *      Author: vanlo
 */
#include "pid_control.h"
#include <stdlib.h>

#define _impl(x) ((pi_t*)(x))
#define PID_PI 3.14159265358979323846f

typedef struct pi_para {
    float_t kp;
    float_t ki;
    float_t kd;
    float_t kc;

    float_t td;       // Sample time (seconds)
    float_t kitd;     // Ki * dt

    float_t output_limit;
    float_t integral_limit;
    float_t d_cutoff_hz;
} pi_para_t;

typedef struct pi {
    float_t i_prev;
    float_t output;

    float_t prev_rate;
    float_t d_filtered;
    uint8_t d_initialized;

    pi_para_t m_para;
} pi_t;


static inline float_t pi_limit(float_t value, float_t limit) {
	return clampf(value, -limit, limit);
}

static inline uint8_t pi_is_clamping(float_t error, float_t sat_input,
		float_t sat_output) {
	float_t sat_error = sat_input - sat_output;

	return (sat_error * error > 0.0f);
}

pid_control_t* sm_pid_control_init(const float_t initial_value, const float_t ki,
		const float_t kp, const float_t kc, const float_t td) {
	if (!isfinite(initial_value) || !isfinite(ki) || !isfinite(kp)
			|| !isfinite(kc) || !isfinite(td) || td <= 0.0f)
		return NULL;

	pi_t *this = (pi_t*) calloc(1, sizeof(pi_t));
	if (!this)
		return NULL;
	this->m_para.kp = kp;
	this->m_para.ki = ki;
	this->m_para.kd = 0.0f;
	this->m_para.kc = kc;

	this->m_para.td = td;
	this->m_para.kitd = ki * td;

	this->m_para.output_limit = 1.0e6f;
	this->m_para.integral_limit = 1.0e6f;
	this->m_para.d_cutoff_hz = 0.0f;

	this->i_prev = initial_value;
	this->output = initial_value;

	return (pid_control_t*) this;
}

void sm_pid_control_reset(pid_control_t *_this) {
	if (!_this)
		return;

	pi_t *this = _impl(_this);

	this->i_prev = 0.0f;
	this->output = 0.0f;

	this->prev_rate = 0.0f;
	this->d_filtered = 0.0f;
	this->d_initialized = 0;
}

void sm_pid_control_deint(pid_control_t *_this) {
	if (!_this)
		return;

	free(_this);
}

void sm_pid_control_para(pid_control_t *_this, const float_t ki, const float_t kp,
		float kc, const float_t td) {
	if (!_this || !isfinite(ki) || !isfinite(kp) || !isfinite(kc)
			|| !isfinite(td) || td <= 0.0f)
		return;

	pi_t *this = _impl(_this);

	this->m_para.kp = kp;
	this->m_para.ki = ki;
	this->m_para.kc = kc;
	this->m_para.td = td;
	this->m_para.kitd = ki * td;
}

void sm_pid_control_config(pid_control_t *_this, float_t kd, float_t output_limit,
		float_t integral_limit, float_t d_cutoff_hz) {

	if (!_this || !isfinite(kd) || !isfinite(output_limit)
			|| !isfinite(integral_limit) || !isfinite(d_cutoff_hz)
			|| output_limit <= 0.0f || integral_limit < 0.0f
			|| d_cutoff_hz < 0.0f)
		return;

	pi_t *this = _impl(_this);

	this->m_para.kd = kd;
	this->m_para.output_limit = output_limit;
	this->m_para.integral_limit = integral_limit;
	this->m_para.d_cutoff_hz = d_cutoff_hz;

	this->i_prev = pi_limit(this->i_prev, integral_limit);

	this->d_filtered = 0.0f;
	this->d_initialized = 0;
}

float_t sm_pi_running(pid_control_t *_this, float_t f_error, float sat_input,
		float sat_output) {

	if (!_this || !isfinite(f_error) ||
	!isfinite(sat_input) ||
	!isfinite(sat_output))
		return 0.0f;

	pi_t *this = _impl(_this);
	pi_para_t *para = &this->m_para;
	float_t f_P = para->kp * f_error;

	// Back-calculation correction
	float_t sat_error = sat_input - sat_output;
	float_t f_I = para->kitd * f_error - para->kc * para->td * sat_error;
	this->i_prev = pi_limit(this->i_prev + f_I, para->integral_limit);
	this->output = pi_limit(this->i_prev + f_P, para->output_limit);
	return this->output;
}

// PI: Conditional integration anti-windup
float_t sm_pi_clamping_running(pid_control_t *_this, float f_error,
		float sat_input, float sat_output) {
	if (!_this || !isfinite(f_error) ||
	!isfinite(sat_input) ||
	!isfinite(sat_output))
		return 0.0f;

	pi_t *this = _impl(_this);
	pi_para_t *para = &this->m_para;

	float_t f_P = para->kp * f_error;
	float_t f_I = para->kitd * f_error;
	float_t i_new = pi_limit(this->i_prev + f_I, para->integral_limit);
	float_t unsat = f_P + i_new;
	uint8_t clamping = pi_is_clamping(f_error, sat_input, sat_output);

	if (unsat > para->output_limit && f_error > 0.0f)
		clamping = 1;
	if (unsat < -para->output_limit && f_error < 0.0f)
		clamping = 1;
	if (!clamping)
		this->i_prev = i_new;
	this->output = pi_limit(f_P + this->i_prev, para->output_limit);
	return this->output;
}

float_t sm_pid_running(pid_control_t *_this, float_t _sp,float_t _fb) {

	if (!_this || !isfinite(_sp) || !isfinite(_fb))
		return 0.0f;
	pi_t *this = _impl(_this);
	pi_para_t *para = &this->m_para;
	float_t dt = para->td;

	if (dt <= 0.0f)
		return 0.0f;

	float_t error = _sp - _fb;
	float_t f_P = para->kp * error;
	float_t derivative = 0.0f;

	if (this->d_initialized) {
		derivative = -(_fb - this->prev_rate) / dt;
	}

	this->prev_rate = _fb;
	this->d_initialized = 1;

	float_t f_D = 0.0f;

	if (para->d_cutoff_hz > 0.0f && para->kd != 0.0f) {

		float_t tau = 1.0f / (2.0f * PID_PI * para->d_cutoff_hz);
		float_t alpha = tau / (tau + dt);
		this->d_filtered = alpha * this->d_filtered + (1.0f - alpha) * derivative;
		f_D = para->kd * this->d_filtered;
	} else {
		this->d_filtered = 0.0f;
	}

	// I term
	float_t i_new = pi_limit(this->i_prev + para->kitd * error,para->integral_limit);

	// Anti-windup
	float_t unsat = f_P + i_new + f_D;
	uint8_t clamping = 0;
	if (unsat > para->output_limit && error > 0.0f) clamping = 1;
	if (unsat < -para->output_limit && error < 0.0f) clamping = 1;
	if (!clamping) this->i_prev = i_new;

	this->output = pi_limit(f_P + this->i_prev + f_D, para->output_limit);
	return this->output;
}
