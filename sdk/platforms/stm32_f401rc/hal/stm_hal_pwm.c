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
	stm_pwm_t* this = (stm_pwm_t*)malloc(sizeof(stm_pwm_t));
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

int32_t sm_hal_pwm_add_channel(sm_hal_pwm_t* _this,PWM_CHANNEL channel){

	if(!_this || channel >= PWM_CHANNEL_NUMBER) return -1;
	_impl(_this)->m_pwm_channel[channel].m_channel =  0x1UL << channel;
	_impl(_this)->m_pwm_channel[channel].m_ccr =  0;
	_impl(_this)->m_pwm_channel_counter ++;
	return 0;
}

int32_t sm_hal_pwm_start(sm_hal_pwm_t* _this){

	if(!_this) return -1;
	for(uint8_t i = 0; i < _impl(_this)->m_pwm_channel_counter; i ++){

		LL_TIM_CC_EnableChannel(_impl(_this)->m_pwm, _impl(_this)->m_pwm_channel[i].m_channel);
	}
	LL_TIM_EnableAllOutputs(_impl(_this)->m_pwm);
	LL_TIM_EnableCounter(_impl(_this)->m_pwm);
	return 0;
}
int32_t sm_hal_pwm_stop(sm_hal_pwm_t* _this){

	if(!_this) return -1;
	LL_TIM_DisableAllOutputs(_impl(_this)->m_pwm);
	return 0;
}

int32_t sm_hal_pwm_set_frequency(sm_hal_pwm_t* _this, uint32_t frequency){

    if (!_this || frequency == 0)
        return -1;
    stm_pwm_t* this = _impl(_this);
    TIM_TypeDef* tim = this->m_pwm;

    uint32_t mode = LL_TIM_GetCounterMode(tim);
    uint32_t center = (mode != LL_TIM_COUNTERMODE_UP && mode != LL_TIM_COUNTERMODE_DOWN);

    uint32_t psc = tim->PSC + 1U;
    uint64_t div = (uint64_t)frequency * psc *(center ? 2U : 1U);
    uint64_t count = (uint64_t)this->m_cnt_clock / div;

    if (count < 2U)
        return -1;
    uint64_t arr = center ? count : count - 1U;
    if (arr > 0xFFFFU)
        return -1;
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
int32_t sm_hal_pwm_set_duty(sm_hal_pwm_t* _this,uint32_t channel ,float duty){

	if (!_this)return -1;
	float duty_sat = SAT(duty,_impl(_this)->m_min_duty,_impl(_this)->m_max_duty);
	_impl(_this)->m_pwm_channel[channel].m_ccr = (uint16_t)(duty_sat*_impl(_this)->m_max_counter);
	TIM_TypeDef* tim = _impl(_this)->m_pwm;
	switch (0x1UL << channel) {
	case LL_TIM_CHANNEL_CH1:
		LL_TIM_OC_SetCompareCH1(tim, _impl(_this)->m_pwm_channel[channel].m_ccr);
		break;

	case LL_TIM_CHANNEL_CH2:
		LL_TIM_OC_SetCompareCH2(tim, _impl(_this)->m_pwm_channel[channel].m_ccr);
		break;

	case LL_TIM_CHANNEL_CH3:
		LL_TIM_OC_SetCompareCH3(tim, _impl(_this)->m_pwm_channel[channel].m_ccr);
		break;

	case LL_TIM_CHANNEL_CH4:
		LL_TIM_OC_SetCompareCH4(tim, _impl(_this)->m_pwm_channel[channel].m_ccr);
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



