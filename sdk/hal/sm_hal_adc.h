/*
 * sm_hal_adc.h
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_ADC_H_
#define SDK_HAL_SM_HAL_ADC_H_
#include "sm_types.h"

typedef enum {
	ADC_TRIGGER_RISING 		= 0,
	ADC_TRIGGER_FALLING		= 1,
	ADC_TRIGGER_RISINGFALLING	= 2
}ADC_TRIGGER_EDGE;

typedef void sm_hal_adc_t;
typedef void (*sm_hal_adc_cb_fn_t)(void*);

sm_hal_adc_t* sm_hal_adc_init(void* adc_hanlde, uint32_t _interupt, uint8_t _priority);
int32_t sm_hal_adc_deinit(sm_hal_adc_t* _this);

int32_t sm_hal_adc_add_channel(sm_hal_adc_t *_this, uint32_t _channel,uint32_t _rank,
		uint32_t _samplingtime, uint32_t _singlediff);

int32_t sm_hal_adc_start(sm_hal_adc_t* _this);
int32_t sm_hal_adc_stop(sm_hal_adc_t* _this);

void sm_hal_adc_irq(sm_hal_adc_t* _this);
int32_t sm_hal_adc_set_callback(sm_hal_adc_t* _this,sm_hal_adc_cb_fn_t _cb, void* _arg);

int32_t sm_hal_adc_set_adc_trigger_edge(sm_hal_adc_t* _this, ADC_TRIGGER_EDGE _edge);

int32_t sm_hal_adc_det_adc_value(sm_hal_adc_t *_this,uint8_t _adc_index);
#endif /* SDK_HAL_SM_HAL_ADC_H_ */
