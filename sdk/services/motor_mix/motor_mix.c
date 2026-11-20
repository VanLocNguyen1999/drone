/*
 * motor_mix.c
 *
 *  Created on: Nov 18, 2026
 *      Author: vanlo
 */

#include "motor_mix.h"

void sm_motor_mixer_update(const sm_mixer_input_t *input,
		sm_mixer_output_t *output) {
	if (!input || !output)
		return;

	float t = clampf(input->throttle, 0.0f, 1.0f);

	float r = input->roll;
	float p = input->pitch;
	float y = input->yaw;

	// X configuration:
	// M1: Front Left  - CCW
	// M2: Front Right - CW
	// M3: Rear Right  - CCW
	// M4: Rear Left   - CW
	float m1 = t + r - p - y;
	float m2 = t - r - p + y;
	float m3 = t - r + p - y;
	float m4 = t + r + p + y;

	output->motor[0] = clampf(m1, 0.0f, 1.0f);
	output->motor[1] = clampf(m2, 0.0f, 1.0f);
	output->motor[2] = clampf(m3, 0.0f, 1.0f);
	output->motor[3] = clampf(m4, 0.0f, 1.0f);
}




