/*
 * stm_hal_io.c
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */
#include "sm_hal_io.h"
#include "gpio.h"


typedef struct sm_io_impl {

	void* m_gpio;
	uint16_t pin;
	sm_hal_io_mode_t mode;
}sm_io_impl_t;

#define _impl(x)		((sm_io_impl_t*)(x))
sm_hal_io_t* sm_hal_io_init(void* _hanlde, uint16_t pin){

	if(!_hanlde) return NULL;
	sm_io_impl_t* this =  (sm_io_impl_t*) malloc(sizeof(sm_io_impl_t));
	this->m_gpio = _hanlde;
	this->pin = pin;

	return (sm_hal_io_t*)this;
}

int32_t sm_hal_io_deinit(sm_hal_io_t* _this){

	if(!_this) return -1;
	free(_this);
	return 0;
}

int32_t sm_hal_io_read(sm_hal_io_t* _this){

	if(!_this) return -1;

	GPIO_PinState ret = HAL_GPIO_ReadPin(_impl(_this)->m_gpio,_impl(_this)->pin);
	return ret;
}
int32_t sm_hal_io_write(sm_hal_io_t* _this, sm_hal_io_status_t status){

	if(!_this) return -1;
	HAL_GPIO_WritePin(_impl(_this)->m_gpio,_impl(_this)->pin, status);
	return 0;
}

int32_t sm_hal_io_open(sm_hal_io_t* _this, sm_hal_io_mode_t mode){

	if(!_this) return -1;
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	uint32_t gpio_mode = GPIO_MODE_INPUT;
	uint32_t gpio_pull = GPIO_PULLDOWN;
	uint32_t gpio_speed = 0;
	switch (mode) {
		case SM_HAL_IO_INPUT_NOPULL:
			gpio_pull = GPIO_NOPULL;
			break;
		case SM_HAL_IO_INPUT_PULLUP:
			gpio_pull = GPIO_PULLUP;
			break;
		case SM_HAL_IO_INPUT_PULLDOWN:
			gpio_pull = GPIO_PULLDOWN;
			break;
		case SM_HAL_IO_OUTPUT_PP:
			gpio_mode = GPIO_MODE_OUTPUT_PP;
			gpio_speed = GPIO_SPEED_FREQ_HIGH;
			break;
		case SM_HAL_IO_OUTPUT_OD:
			gpio_mode = GPIO_MODE_OUTPUT_OD;
			gpio_speed = GPIO_SPEED_FREQ_HIGH;
			break;
		default:
			return -1;
	}
	GPIO_InitStruct.Pin = _impl(_this)->pin;
	GPIO_InitStruct.Mode = gpio_mode;
	GPIO_InitStruct.Pull = gpio_pull;
	GPIO_InitStruct.Speed = gpio_speed;
	HAL_GPIO_Init(_impl(_this)->m_gpio, &GPIO_InitStruct);
	return 0;
}
int32_t sm_hal_io_close(sm_hal_io_t* _this){

	if(!_this) return -1;
	HAL_GPIO_DeInit(_impl(_this)->m_gpio, _impl(_this)->pin);
	return 0;
}




