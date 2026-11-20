/*
 * stm_hal_pwm.c
 *
 *  Created on: Nov 13, 2026
 *      Author: vanlo
 */

#include "sm_hal_pwm.h"
#include "stm32f4xx_ll_tim.h"

#define MAX_PWM_CHANNEL		4
#define _impl(x)			((stm_pwm_t*)(x))

typedef struct sm_pwm_channel {

	uint32_t m_channel;
	uint32_t m_ccr;
}stm_pwm_channel_t;

typedef struct stm_pwm {

	TIM_TypeDef* m_pwm;
	uint32_t m_frequency;
	uint32_t m_deadtime_ns;
	uint32_t m_cnt_clock;
	float m_max_duty;
	float m_min_duty;
	uint32_t m_max_counter;
	stm_pwm_channel_t m_pwm_channel[MAX_PWM_CHANNEL];
	uint8_t m_pwm_channel_counter;
}stm_pwm_t;

sm_hal_pwm_t* sm_hal_pwm_init(void* pwm_hanlde,uint32_t sys_frequency, float _max_duty, float _min_duty){

	if(!pwm_hanlde) return NULL;
	stm_pwm_t* this = (stm_pwm_t*)calloc(1, sizeof(stm_pwm_t));
	if (!this) return NULL;
	this->m_pwm = pwm_hanlde;
	this->m_max_duty = _max_duty;
	this->m_min_duty = _min_duty;
	this->m_cnt_clock = sys_frequency;
	this->m_max_counter = this->m_pwm->ARR;
	this->m_pwm_channel_counter = 0;
	return this;
}
int32_t sm_hal_pwm_deinit(sm_hal_pwm_t* _this){

	if(!_this) return -1;
	free(_this);
	return 0;
}

int32_t sm_hal_pwm_add_channel(sm_hal_pwm_t* _this, PWM_CHANNEL channel) {
    if (!_this || channel < PWM_CHANNEL_1 || channel > PWM_CHANNEL_4) return -1;
    stm_pwm_t* this = _impl(_this);
    if (this->m_pwm_channel_counter >= MAX_PWM_CHANNEL) return -1;

    uint32_t channel_mask;
    switch (channel) {
    case PWM_CHANNEL_1: channel_mask = LL_TIM_CHANNEL_CH1; break;
    case PWM_CHANNEL_2: channel_mask = LL_TIM_CHANNEL_CH2; break;
    case PWM_CHANNEL_3: channel_mask = LL_TIM_CHANNEL_CH3; break;
    case PWM_CHANNEL_4: channel_mask = LL_TIM_CHANNEL_CH4; break;
    default: return -1;
    }

    for (uint8_t i = 0; i < this->m_pwm_channel_counter; i++) {
        if (this->m_pwm_channel[i].m_channel == channel_mask) return -1;
    }

    this->m_pwm_channel[this->m_pwm_channel_counter].m_channel = channel_mask;
    this->m_pwm_channel[this->m_pwm_channel_counter].m_ccr = 0;
    this->m_pwm_channel_counter++;
    return 0;
}

int32_t sm_hal_pwm_start(sm_hal_pwm_t* _this) {
    if (!_this) return -1;
    stm_pwm_t* this = _impl(_this);
    TIM_TypeDef* tim = this->m_pwm;
    /* Load preloaded CCR values before enabling the timer */
    LL_TIM_GenerateEvent_UPDATE(tim);
    for (uint8_t i = 0; i < this->m_pwm_channel_counter; i++) {
        LL_TIM_CC_EnableChannel(tim, this->m_pwm_channel[i].m_channel);
    }
    /* BDTR/MOE is needed on advanced timers, not on general-purpose TIM2. */
    if (IS_TIM_BREAK_INSTANCE(tim)) LL_TIM_EnableAllOutputs(tim);
    LL_TIM_EnableCounter(tim);
    return 0;
}
int32_t sm_hal_pwm_stop(sm_hal_pwm_t* _this) {
    if (!_this) return -1;
    stm_pwm_t* this = _impl(_this);
    TIM_TypeDef* tim = this->m_pwm;
    LL_TIM_DisableCounter(tim);
    for (uint8_t i = 0; i < this->m_pwm_channel_counter; i++) {
        LL_TIM_CC_DisableChannel(tim, this->m_pwm_channel[i].m_channel);
    }
    if (IS_TIM_BREAK_INSTANCE(tim)) LL_TIM_DisableAllOutputs(tim);
    return 0;
}

int32_t sm_hal_pwm_set_frequency(sm_hal_pwm_t* _this, uint32_t frequency) {
    if (!_this || frequency == 0U) return -1;
    stm_pwm_t* this = _impl(_this);
    TIM_TypeDef* tim = this->m_pwm;

    uint32_t mode = LL_TIM_GetCounterMode(tim);
    uint32_t center = (mode != LL_TIM_COUNTERMODE_UP && mode != LL_TIM_COUNTERMODE_DOWN);
    uint32_t psc = tim->PSC + 1U;
    uint64_t div = (uint64_t)frequency * psc * (center ? 2U : 1U);
    uint64_t count = (uint64_t)this->m_cnt_clock / div;

    if (count < 2U) return -1;
    uint64_t arr = center ? count : count - 1U;
    /* TIM2 is 32-bit on STM32F401. Other timers may be 16-bit. */
    uint64_t max_arr = (tim == TIM2) ? 0xFFFFFFFFULL : 0xFFFFULL;
    if (arr > max_arr) return -1;

    LL_TIM_SetAutoReload(tim, (uint32_t)arr);
    this->m_max_counter = (uint32_t)arr;
    this->m_frequency = frequency;
    return 0;
}
int32_t sm_hal_pwm_set_deadtime(sm_hal_pwm_t* _this, uint32_t deadtime_ns){

	if(!_this) return -1;
	_impl(_this)->m_deadtime_ns = deadtime_ns;

	uint32_t result = (uint32_t) (((uint64_t) deadtime_ns *_impl(_this)->m_cnt_clock) / 1000000000ULL);
	if(result >= 128 && result <= 255){
		if(result > 254) result = 254;
		result = result/2 + 64;
	}
	else if(result >= 256 && result <= 511){
		if(result > 504) result = 504;
		result = result/8 + 160;
	}
	else if(result >= 512){
		if(result > 1008) result = 1008;
		result = result/16 + 192;
	}
	LL_TIM_OC_SetDeadTime(_impl(_this)->m_pwm, result);
	return 0;
}
int32_t sm_hal_pwm_set_duty(sm_hal_pwm_t* _this, uint32_t channel, float duty) {
    if (!_this || channel < PWM_CHANNEL_1 || channel > PWM_CHANNEL_4) return -1;
    stm_pwm_t* this = _impl(_this);
    uint32_t channel_mask;
    switch (channel) {
    case PWM_CHANNEL_1: channel_mask = LL_TIM_CHANNEL_CH1; break;
    case PWM_CHANNEL_2: channel_mask = LL_TIM_CHANNEL_CH2; break;
    case PWM_CHANNEL_3: channel_mask = LL_TIM_CHANNEL_CH3; break;
    case PWM_CHANNEL_4: channel_mask = LL_TIM_CHANNEL_CH4; break;
    default: return -1;
    }
    int channel_index = -1;
    for (uint8_t i = 0; i < this->m_pwm_channel_counter; i++) {
        if (this->m_pwm_channel[i].m_channel == channel_mask) {
            channel_index = (int)i;
            break;
        }
    }
	if (channel_index < 0)
		return -1;

	TIM_TypeDef *tim = this->m_pwm;

	float duty_sat = SAT(duty, this->m_max_duty, this->m_min_duty);
	uint64_t period = (uint64_t) tim->ARR + 1ULL;
	uint32_t ccr = (uint32_t) ((double) period * (double) duty_sat + 0.5);
	this->m_pwm_channel[channel_index].m_ccr = ccr;

	switch (channel) {
	case PWM_CHANNEL_1:
		LL_TIM_OC_SetCompareCH1(tim, ccr);
		break;
	case PWM_CHANNEL_2:
		LL_TIM_OC_SetCompareCH2(tim, ccr);
		break;
	case PWM_CHANNEL_3:
		LL_TIM_OC_SetCompareCH3(tim, ccr);
		break;
	case PWM_CHANNEL_4:
		LL_TIM_OC_SetCompareCH4(tim, ccr);
		break;
	default:
		return -1;
	}
	return 0;
}

int32_t sm_hal_pwm_enable_update_duty(sm_hal_pwm_t* _this){

	if(!_this) return -1;
	LL_TIM_DisableUpdateEvent(_impl(_this)->m_pwm);
	return 0;
}
int32_t sm_hal_pwm_disable_update_duty(sm_hal_pwm_t* _this){

	if(!_this) return -1;
	LL_TIM_EnableUpdateEvent(_impl(_this)->m_pwm);
	return 0;
}



