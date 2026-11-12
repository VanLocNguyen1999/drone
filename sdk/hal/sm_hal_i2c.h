/*
 * sm_hal_i2c.h
 *
 *  Created on: Nov 10, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_I2C_H_
#define SDK_HAL_SM_HAL_I2C_H_
#include "sm_types.h"
#include "stm32f4xx_hal.h"
typedef void sm_hal_i2c_t;

sm_hal_i2c_t* sm_hal_i2c_init(void* _channel, uint16_t _addr);
void sm_hal_i2c_deinit(sm_hal_i2c_t *_this);

int32_t sm_hal_i2c_set_addr(sm_hal_i2c_t *_this, uint16_t _addr);
int32_t sm_hal_i2c_open(sm_hal_i2c_t *_this);
int32_t sm_hal_i2c_close(sm_hal_i2c_t *_this);
int32_t sm_hal_i2c_write(sm_hal_i2c_t *_this, uint16_t mem_address,
		uint16_t mem_addsize, uint8_t *_data, uint32_t _len,
		uint32_t time_out);
int32_t sm_hal_i2c_read(sm_hal_i2c_t *_this, uint16_t mem_address,
		uint16_t mem_addsize, uint8_t *_data, uint32_t _len, uint32_t time_out);
int32_t sm_hal_i2c_write_read(sm_hal_i2c_t *_this,
                              const uint8_t *_data,
                              uint32_t _data_len,
                              uint8_t* _buf,
                              uint32_t* _recv_len);
#endif /* SDK_HAL_SM_HAL_I2C_H_ */
