/*
 * sm_hal_adc.c
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */
#include "sm_hal_adc.h"
#include "stm32f4xx_hal.h"

#define MAX_ADC_CHANNEL		12

#define _impl(x)			((sm_adc_t*)(x))
typedef struct sm_adc_channel {

	uint32_t m_channel;
	uint32_t m_rank;
	uint32_t m_samplingtime;
	uint32_t m_singlediff;
}sm_adc_channel_t;

typedef struct sm_adc {

	ADC_HandleTypeDef* m_adc;
	uint32_t m_interupt;
	uint8_t m_priority;
	uint32_t m_channel_num;

	sm_adc_channel_t m_adc_channel[MAX_ADC_CHANNEL]	;
	uint32_t m_adc_buffer[MAX_ADC_CHANNEL];

	sm_hal_adc_cb_fn_t m_cb;
	void* m_arg;
}sm_adc_t;

sm_hal_adc_t* sm_hal_adc_init(void* adc_hanlde, uint32_t _interupt, uint8_t _priority){

	if(!adc_hanlde) return NULL;
	sm_adc_t* this = (sm_adc_t*)malloc(sizeof(sm_adc_t));
	this->m_adc = adc_hanlde;
	this->m_interupt = _interupt;
	this->m_priority = _priority;
	this->m_channel_num = 0;
	return (sm_hal_adc_t*)this;
}
int32_t sm_hal_adc_deinit(sm_hal_adc_t* _this){

	if(!_this) return -1;
	free(_this);
	return 0;
}

int32_t sm_hal_adc_add_channel(sm_hal_adc_t *_this,uint32_t _channel, uint32_t _rank,
        uint32_t _samplingtime, uint32_t _singlediff){

    if (!_this) return -1;
    uint32_t i;
    uint32_t channel_num = _impl(_this)->m_channel_num;

    for (i = 0; i < channel_num; i++) {
        if (_impl(_this)->m_adc_channel[i].m_channel == _channel) {
            break;
        }
    }
    if (i == channel_num && channel_num >= MAX_ADC_CHANNEL) {
        return -1;
    }

    ADC_ChannelConfTypeDef config = {0};
    config.Channel = _channel;
    config.Rank = _rank;
    config.SamplingTime = _samplingtime;
    config.Offset = 0;

    if (HAL_ADC_ConfigChannel(_impl(_this)->m_adc, &config) != HAL_OK) {
        return -1;
    }

    _impl(_this)->m_adc_channel[i].m_channel = _channel;
    _impl(_this)->m_adc_channel[i].m_rank = _rank;
    _impl(_this)->m_adc_channel[i].m_samplingtime = _samplingtime;
    _impl(_this)->m_adc_channel[i].m_singlediff = _singlediff;

    if (i == channel_num) {
        _impl(_this)->m_channel_num++;
    }
    return 0;
}

int32_t sm_hal_adc_start(sm_hal_adc_t *_this) {

	if (!_this)
		return -1;
	/* Start ADC */
	if (HAL_ADC_Start_DMA(_impl(_this)->m_adc, _impl(_this)->m_adc_buffer,
			_impl(_this)->m_channel_num) != HAL_OK) {
		/* Calibration Error */
		return -1;
	}
	/* interrupt */
	NVIC_SetPriority(_impl(_this)->m_interupt,
			NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
					_impl(_this)->m_priority, 0));
	NVIC_EnableIRQ(_impl(_this)->m_interupt);
	return 0;
}
int32_t sm_hal_adc_stop(sm_hal_adc_t* _this){

	if (!_this) return -1;
    if (HAL_ADC_Stop_DMA(_impl(_this)->m_adc) != HAL_OK)
        return -1;
    return 0;
}

void sm_hal_adc_irq(sm_hal_adc_t* _this){

	if(!_this ) return;
	if(_impl(_this)->m_cb){
		_impl(_this)->m_cb(_impl(_this)->m_arg);
	}
}
int32_t sm_hal_adc_set_callback(sm_hal_adc_t* _this,sm_hal_adc_cb_fn_t _cb, void* _arg){

	if(!_this || !_cb) return -1;
	_impl(_this)->m_cb = _cb;
	_impl(_this)->m_arg = _arg;
	return 0;
}

int32_t sm_hal_adc_set_adc_trigger_edge(sm_hal_adc_t *_this,ADC_TRIGGER_EDGE _edge) {

	if (!_this) return 0;

	LL_ADC_REG_StopConversionExtTrig(_impl(_this)->m_adc);
	switch (_edge) {
		case ADC_TRIGGER_RISING:
			LL_ADC_REG_StartConversionExtTrig(_impl(_this)->m_adc, LL_ADC_REG_TRIG_EXT_RISING);
			break;
		case ADC_TRIGGER_FALLING:
			LL_ADC_REG_StartConversionExtTrig(_impl(_this)->m_adc, LL_ADC_REG_TRIG_EXT_FALLING);
			break;
		case ADC_TRIGGER_RISINGFALLING:
			LL_ADC_REG_StartConversionExtTrig(_impl(_this)->m_adc, LL_ADC_REG_TRIG_EXT_RISINGFALLING);
			break;
	default:
			return -1;
			break;
		}
	return 0;
}

int32_t sm_hal_adc_det_adc_value(sm_hal_adc_t *_this,uint8_t _adc_index) {

	if (!_this || _adc_index >= _impl(_this)->m_channel_num) return 0;
	return _impl(_this)->m_adc_buffer[_adc_index];
}



