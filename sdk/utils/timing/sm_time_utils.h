/*
 * sm_time_utils.h
 *
 *  Created on: Nov 16, 2026
 *      Author: vanlo
 */

#ifndef SDK_UTILS_TIMING_SM_TIME_UTILS_H_
#define SDK_UTILS_TIMING_SM_TIME_UTILS_H_
#include <stdint.h>
#include <time.h>

extern int64_t get_tick_count();

static inline volatile int64_t getTickCount() {
    return get_tick_count();
}

static inline void delay(int32_t _second) {
    int64_t timeout = getTickCount() + _second * 1000;
    while (timeout > getTickCount());
}

static inline void delayMs(int32_t _ms) {
    volatile int64_t timeout = getTickCount() + _ms;
    while (timeout > getTickCount());
}
#endif /* SDK_UTILS_TIMING_SM_TIME_UTILS_H_ */
