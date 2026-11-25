/*
 * sm_app_bsp.c
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_PLATFORMS_STM32_F401RC_APP_BSP_SM_APP_BSP_C_
#define SDK_PLATFORMS_STM32_F401RC_APP_BSP_SM_APP_BSP_C_
#include "app_bsp.h"
#include "stm32f4xx_hal.h"

#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

sm_hal_i2c_t *g_mpu_i2c;
sm_hal_flash_t *g_flash;
sm_hal_io_t *g_led;
sm_hal_pwm_t *g_pwm2;
sm_hal_uart_t *g_uart1;

static void sm_bsp_io_init(void);
static void sm_bsp_i2c_init(void);
static void sm_bsp_flash_init(void);
static void sm_bsp_pwm_init(void);
static void sm_bsp_uart_init(void);
void sm_bsp_init(void){

	sm_bsp_io_init();
	sm_bsp_i2c_init();
	sm_bsp_flash_init();
	sm_bsp_pwm_init();
	sm_bsp_uart_init();
}

static void sm_bsp_io_init(void){

	g_led = sm_hal_io_init(GPIOC, GPIO_PIN_13);
	sm_hal_io_open(g_led,SM_HAL_IO_OUTPUT_PP);
	sm_hal_io_write(g_led, SM_HAL_IO_RESET);
}
sm_hal_io_t * sm_bsp_get_io_led(void){

	return g_led;
}

static void sm_bsp_i2c_init(void){
	g_mpu_i2c = sm_hal_i2c_init(&hi2c1, 0);
	sm_hal_i2c_open(g_mpu_i2c);
}
sm_hal_i2c_t * sm_bsp_get_mpu_i2c(void){

	return g_mpu_i2c;
}

static void sm_bsp_flash_init(void){
	g_flash = sm_hal_flash_init(NULL);
}
sm_hal_flash_t * sm_bsp_get_flash(void){

	return g_flash;
}

static void sm_bsp_pwm_init(void){
	g_pwm2 = sm_hal_pwm_init(TIM2, 84000000UL, 0.8f, 0.0f);

	sm_hal_pwm_add_channel(g_pwm2, PWM_CHANNEL_1);
	sm_hal_pwm_add_channel(g_pwm2, PWM_CHANNEL_2);
	sm_hal_pwm_add_channel(g_pwm2, PWM_CHANNEL_3);
	sm_hal_pwm_add_channel(g_pwm2, PWM_CHANNEL_4);
	sm_hal_pwm_stop(g_pwm2);

	sm_hal_pwm_set_frequency(g_pwm2, 500);
	sm_hal_pwm_set_duty(g_pwm2, PWM_CHANNEL_1, 0.0f);
	sm_hal_pwm_set_duty(g_pwm2, PWM_CHANNEL_2, 0.0f);
	sm_hal_pwm_set_duty(g_pwm2, PWM_CHANNEL_3, 0.0f);
	sm_hal_pwm_set_duty(g_pwm2, PWM_CHANNEL_4, 0.0f);
	sm_hal_pwm_start(g_pwm2);
}
sm_hal_pwm_t *sm_bsp_get_pwm2(void){

	return g_pwm2;;
}

static void sm_bsp_uart_init(void){

	g_uart1 = sm_hal_uart_init_ex(&huart1, 9600, 1, 8, USART1_IRQn);
	sm_hal_uart_start(g_uart1);
}

sm_hal_uart_t *sm_bsp_get_uart1(void){

	return g_uart1;;
}

#endif /* SDK_PLATFORMS_STM32_F401RC_APP_BSP_SM_APP_BSP_C_ */
