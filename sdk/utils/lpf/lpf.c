/*
 * lpf.c
 *
 *  Created on: Nov 11, 2026
 *      Author: vanlo
 */

#include "lpf.h"

void lpf_init(lpf_t *_this, float _cutOffFrequency, float _freq_sample_rate){
	_this->output = 0;
	_this->epow = 1-exp(-(1/_freq_sample_rate) * 2 * M_PI * _cutOffFrequency);
}

float lpf_update(lpf_t *_this,float _input){
	_this->output += (_input - _this->output) * _this->epow;
	return lpf_update(_this);
}


