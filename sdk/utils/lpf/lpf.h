/*
 * lpf.h
 *
 *  Created on: Nov 11, 2026
 *      Author: vanlo
 */

#ifndef SDK_UTILS_LPF_LPF_H_
#define SDK_UTILS_LPF_LPF_H_
#include "sm_types.h"

typedef struct lpf lpf_t;

struct lpf{
	float output;
	float epow;
};
void lpf_init(lpf_t *_this,float _cutOffFrequency, float _freq_sample_rate);
float lpf_update(lpf_t *_this,float _input);
static inline float lpf_get_update(lpf_t *_this){
	return _this->output;
}
#endif /* SDK_UTILS_LPF_LPF_H_ */
