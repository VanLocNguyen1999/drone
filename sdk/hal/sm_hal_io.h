/*
 * sm_hal_io.h
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_IO_H_
#define SDK_HAL_SM_HAL_IO_H_
#include "sm_types.h"

typedef enum {
	SM_HAL_IO_RESET = 0,
	SM_HAL_IO_SET = 1
} sm_hal_io_status_t;

typedef enum {
	SM_HAL_IO_INPUT_NOPULL = 0,
	SM_HAL_IO_INPUT_PULLUP,
	SM_HAL_IO_INPUT_PULLDOWN,
    SM_HAL_IO_OUTPUT_PP,
    SM_HAL_IO_OUTPUT_OD
} sm_hal_io_mode_t;
typedef void sm_hal_io_t;

sm_hal_io_t* sm_hal_io_init(void* _hanlde, uint16_t pin);
int32_t sm_hal_io_deinit(sm_hal_io_t* _this);
int32_t sm_hal_io_read(sm_hal_io_t* _this);
int32_t sm_hal_io_write(sm_hal_io_t* _this, sm_hal_io_status_t status);

int32_t sm_hal_io_open(sm_hal_io_t* _this, sm_hal_io_mode_t mode);
int32_t sm_hal_io_close(sm_hal_io_t* _this);
#endif /* SDK_HAL_SM_HAL_IO_H_ */
