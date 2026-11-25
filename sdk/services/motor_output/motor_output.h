/*
 * motor_output.h
 *
 *  Xuat lenh motor 0..1 ra PWM: ESC (1000..2000 us) hoac duty truc tiep.
 */

#ifndef SDK_SERVICES_MOTOR_OUTPUT_MOTOR_OUTPUT_H_
#define SDK_SERVICES_MOTOR_OUTPUT_MOTOR_OUTPUT_H_
#include "sm_types.h"
#include "sm_hal_pwm.h"

#define MOTOR_OUTPUT_NUM	4

typedef enum {
	MOTOR_OUTPUT_MODE_ESC_US = 0,	/* m = 0..1 -> us_min..us_max */
	MOTOR_OUTPUT_MODE_DUTY,			/* m = 0..1 -> duty 0..1 (motor choi than + MOSFET) */
} MOTOR_OUTPUT_MODE;

typedef struct {
	sm_hal_pwm_t *pwm;
	PWM_CHANNEL channel[MOTOR_OUTPUT_NUM];	/* motor[i] -> kenh TIM */
	uint32_t freq_hz;
	float us_min;
	float us_max;
	MOTOR_OUTPUT_MODE mode;
} sm_motor_output_cfg_t;

typedef void sm_motor_output_t;

sm_motor_output_t* sm_motor_output_init(const sm_motor_output_cfg_t *cfg);

/* m[i] = 0..1. Gia tri ngoai khoang bi clamp, NaN -> 0 */
int32_t sm_motor_output_write(sm_motor_output_t *_this, const float m[MOTOR_OUTPUT_NUM]);

/* Tat motor: ESC -> us_min (van giu xung de ESC khong bao mat tin hieu) */
int32_t sm_motor_output_stop(sm_motor_output_t *_this);

#endif /* SDK_SERVICES_MOTOR_OUTPUT_MOTOR_OUTPUT_H_ */
