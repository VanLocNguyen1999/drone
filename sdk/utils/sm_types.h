/*
 * sm_types.h
 *
 *  Created on: Nov 10, 2026
 *      Author: vanlo
 */

#ifndef SDK_UTILS_SM_TYPES_H_
#define SDK_UTILS_SM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <math.h>
#ifndef uint8_t
typedef uint8_t uint8_t;
#endif

#ifndef int8_t
typedef int8_t int8_t;
#endif

#ifndef uint16_t
typedef uint16_t uint16_t;
#endif

#ifndef int16_t
typedef int16_t int16_t;
#endif

#ifndef uint32_t
typedef uint32_t uint32_t;
#endif

#ifndef int32_t
typedef int32_t int32_t;
#endif

#define PI_NUMBER  (3.14159265359f)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef void* sm_mutex;

#define MUTEX_INIT(x)           (void)(x)

#define ENTER_CRITICAL(x)       (void)(x)
#define EXIT_CRITICAL(x)        (void)(x)

#define MATH_MIN2(a, b)                                       (((a) < (b)) ? (a) : (b))
#define MATH_MAX2(a, b)                                       (((a) > (b)) ? (a) : (b))
#define SAT(input, up, down) 								  (((input) > (up)) ? (up) : (((input) < (down)) ? (down) : (input)))

static inline float clampf(float x, float min, float max) {
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}
#endif /* SDK_UTILS_SM_TYPES_H_ */
