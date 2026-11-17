/*
 * sm_elapsed_timer.h
 *
 *  Created on: Nov 16, 2026
 *      Author: vanlo
 */

#ifndef SDK_UTILS_TIMING_SM_ELAPSED_TIMER_H_
#define SDK_UTILS_TIMING_SM_ELAPSED_TIMER_H_
#include <stdint.h>

int64_t get_tick_count();

typedef struct elapsed_timer{
    int64_t m_duration;
    int64_t m_start_time;
}elapsed_timer_t;

static inline void elapsed_timer_reset(elapsed_timer_t* _self){
    _self->m_start_time = get_tick_count();
}

static inline void elapsed_timer_resetz(elapsed_timer_t* _self, int32_t _duration){
    _self->m_duration = _duration;
    _self->m_start_time = get_tick_count();
}

static inline void elapsed_timer_pass(elapsed_timer_t* _self){
    _self->m_duration = 0;
}

static inline int64_t elapsed_timer_get_remain(elapsed_timer_t* _self){
    int64_t remain_time = get_tick_count() - _self->m_start_time;
    if(remain_time >= _self->m_duration){
        return 0;
    }else{
        return _self->m_duration - remain_time;
    }
}
#endif /* SDK_UTILS_TIMING_SM_ELAPSED_TIMER_H_ */
