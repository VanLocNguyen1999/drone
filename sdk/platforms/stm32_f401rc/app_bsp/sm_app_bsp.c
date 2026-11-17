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
#include "i2c.h"

sm_hal_i2c_t *g_mpu_i2c;
sm_hal_flash_t *g_flash;
sm_hal_io_t * g_led;

static void sm_bsp_io_init(void);
static void sm_bsp_i2c_init(void);
static void sm_bsp_flash_init(void);
void sm_bsp_init(void){

	sm_bsp_io_init();
	sm_bsp_i2c_init();
	sm_bsp_flash_init();
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

#endif /* SDK_PLATFORMS_STM32_F401RC_APP_BSP_SM_APP_BSP_C_ */
