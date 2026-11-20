/*
 * motor_mix.h
 *
 *  Created on: Nov 18, 2026
 *      Author: vanlo
 */

#ifndef SDK_SERVICES_MOTOR_MIX_MOTOR_MIX_H_
#define SDK_SERVICES_MOTOR_MIX_MOTOR_MIX_H_
#include "sm_types.h"

#define MOTOR_COUNT 4

typedef struct {
    float throttle;  // 0.0 -> 1.0
    float roll;      // -1.0 -> 1.0
    float pitch;     // -1.0 -> 1.0
    float yaw;       // -1.0 -> 1.0
} sm_mixer_input_t;

typedef struct {
    float motor[MOTOR_COUNT];
} sm_mixer_output_t;

void sm_motor_mixer_update(const sm_mixer_input_t *input,sm_mixer_output_t *output);
#endif /* SDK_SERVICES_MOTOR_MIX_MOTOR_MIX_H_ */
