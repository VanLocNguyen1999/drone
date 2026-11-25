/*
 * motor_output.c
 */

#include "motor_output.h"

#define _impl(x) ((sm_motor_output_impl_t*)(x))

typedef struct {
	sm_motor_output_cfg_t m_cfg;
	float m_period_us;
	uint8_t m_init;
} sm_motor_output_impl_t;

static sm_motor_output_impl_t g_motor_output;

static float motor_output_to_duty(const sm_motor_output_impl_t *this, float m) {

	if (!isfinite(m))
		m = 0.0f;
	m = clampf(m, 0.0f, 1.0f);

	if (this->m_cfg.mode == MOTOR_OUTPUT_MODE_DUTY)
		return m;

	float us = this->m_cfg.us_min + (this->m_cfg.us_max - this->m_cfg.us_min) * m;
	return us / this->m_period_us;
}

sm_motor_output_t* sm_motor_output_init(const sm_motor_output_cfg_t *cfg) {

	if (!cfg || !cfg->pwm || cfg->freq_hz == 0U)
		return NULL;

	sm_motor_output_impl_t *this = &g_motor_output;
	float period_us = 1.0e6f / (float) cfg->freq_hz;

	if (cfg->mode == MOTOR_OUTPUT_MODE_ESC_US) {
		/* Xung max phai nam trong 1 chu ky PWM */
		if (cfg->us_min <= 0.0f || cfg->us_max <= cfg->us_min || cfg->us_max >= period_us)
			return NULL;
	}

	this->m_cfg = *cfg;
	this->m_period_us = period_us;

	if (sm_hal_pwm_set_frequency(cfg->pwm, cfg->freq_hz) < 0)
		return NULL;
	/* Module tu quan ly gioi han, khong de HAL cat duty */
	sm_hal_pwm_set_duty_sat(cfg->pwm, 1.0f, 0.0f);

	this->m_init = 1;
	sm_motor_output_stop(this);
	return this;
}

int32_t sm_motor_output_write(sm_motor_output_t *_this, const float m[MOTOR_OUTPUT_NUM]) {

	if (!_this || !m || !_impl(_this)->m_init)
		return -1;

	sm_motor_output_impl_t *this = _impl(_this);
	int32_t ret = 0;

	for (uint32_t i = 0; i < MOTOR_OUTPUT_NUM; i++) {
		if (sm_hal_pwm_set_duty(this->m_cfg.pwm, this->m_cfg.channel[i],
				motor_output_to_duty(this, m[i])) < 0)
			ret = -1;
	}
	return ret;
}

int32_t sm_motor_output_stop(sm_motor_output_t *_this) {

	const float zero[MOTOR_OUTPUT_NUM] = { 0 };
	return sm_motor_output_write(_this, zero);
}
