/*
 * stm_hal_time.c
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */
#include "sm_hal_time.h"
#include "stm32f4xx_hal.h"

#define _impl(x)		((stm_time_impl_t*)(x))

typedef struct stm_time_impl {

	void* m_time;
	int32_t m_irq_type;
	uint8_t m_priority;
	sm_hal_timer_cb_fn_t m_cb;
	void* m_arg;
}stm_time_impl_t;

sm_hal_timer_t* sm_hal_timer_init(void* time_hanlde, int32_t time_irq_type, uint8_t priority){

	if(!time_hanlde|| !time_irq_type) return NULL;
	stm_time_impl_t* _this = (stm_time_impl_t*)malloc((sizeof(stm_time_impl_t)));
	_this->m_time = time_hanlde;
	_this->m_irq_type = time_irq_type;
	_this->m_priority = priority;
	return (sm_hal_timer_t*)_this;
}
int32_t sm_hal_timer_deinit(sm_hal_timer_t* _this){

	if(!_this) return -1;
	free(_this);
	return 0;
}

int32_t sm_hal_timer_start(sm_hal_timer_t* _this){

	if(!_this) return -1;
	NVIC_SetPriority(_impl(_this)->m_irq_type,
							NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
							_impl(_this)->m_priority,
							0));
	NVIC_EnableIRQ(_impl(_this)->m_irq_type);
	if (HAL_TIM_Base_Start_IT((TIM_HandleTypeDef*)_impl(_this)->m_time) != HAL_OK) {
		return -1;
	}
	return 0;
}
int32_t sm_hal_timer_stop(sm_hal_timer_t* _this){

	if(!_this) return -1;
	if (HAL_TIM_Base_Stop_IT((TIM_HandleTypeDef*)_impl(_this)->m_time) != HAL_OK) {
		return -1;
	}
	return 0;
}

void sm_hal_timer_irq(sm_hal_timer_t *_this) {

	if (!_this)return;
	if (_impl(_this)->m_cb) {
		_impl(_this)->m_cb(_impl(_this)->m_arg);
	}
}
int32_t sm_hal_timer_set_callback(sm_hal_timer_t* _this,sm_hal_timer_cb_fn_t _cb, void* _arg){

	if (!_this || !_cb) return -1;
	_impl(_this)->m_cb = _cb;
	_impl(_this)->m_arg = _arg;
	return 0;
}




