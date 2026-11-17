/*
 * app_bsp.c
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_BSP_APP_BSP_C_
#define SDK_BSP_APP_BSP_C_
#include "sm_hal.h"

void sm_bsp_init(void);

sm_hal_io_t * sm_bsp_get_io_led(void);
sm_hal_i2c_t * sm_bsp_get_mpu_i2c(void);
sm_hal_flash_t * sm_bsp_get_flash(void);


#endif /* SDK_BSP_APP_BSP_C_ */
