/*
 * sm_hal_pwm.h
 *
 *  Created on: Nov 13, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_PWM_H_
#define SDK_HAL_SM_HAL_PWM_H_
#include "sm_types.h"

typedef void sm_hal_pwm_t;

typedef enum {

	PWM_CHANNEL_1 = 1,
	PWM_CHANNEL_2,
	PWM_CHANNEL_3,
	PWM_CHANNEL_4,
	PWM_CHANNEL_NUMBER
}PWM_CHANNEL;
sm_hal_pwm_t* sm_hal_pwm_init(void* pwm_hanlde,uint32_t sys_frequency, float _max_duty, float _min_duty);
int32_t sm_hal_pwm_deinit(sm_hal_pwm_t* _this);

int32_t sm_hal_pwm_add_channel(sm_hal_pwm_t* _this,PWM_CHANNEL channel);

int32_t sm_hal_pwm_start(sm_hal_pwm_t* _this);
int32_t sm_hal_pwm_stop(sm_hal_pwm_t* _this);

int32_t sm_hal_pwm_set_frequency(sm_hal_pwm_t* _this, uint32_t frequency);
int32_t sm_hal_pwm_set_deadtime(sm_hal_pwm_t* _this, uint32_t deadtime_ns);
int32_t sm_hal_pwm_set_duty(sm_hal_pwm_t* _this,uint32_t channel ,float duty);

int32_t sm_hal_pwm_enable_update_duty(sm_hal_pwm_t* _this);
int32_t sm_hal_pwm_disable_update_duty(sm_hal_pwm_t* _this);
#endif /* SDK_HAL_SM_HAL_PWM_H_ */
