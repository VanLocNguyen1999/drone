/*
 * sm_hal_time.h
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_TIME_H_
#define SDK_HAL_SM_HAL_TIME_H_
#include "sm_types.h"

typedef void sm_hal_timer_t;
typedef void (*sm_hal_timer_cb_fn_t)(void*);

sm_hal_timer_t* sm_hal_timer_init(void* time_hanlde, int32_t time_irq_type, uint8_t priority);
int32_t sm_hal_timer_deinit(sm_hal_timer_t* _this);

int32_t sm_hal_timer_start(sm_hal_timer_t* _this);
int32_t sm_hal_timer_stop(sm_hal_timer_t* _this);

void sm_hal_timer_irq(sm_hal_timer_t* _this);
int32_t sm_hal_timer_set_callback(sm_hal_timer_t* _this,sm_hal_timer_cb_fn_t _cb, void* _arg);
#endif /* SDK_HAL_SM_HAL_TIME_H_ */
