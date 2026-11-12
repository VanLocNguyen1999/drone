/*
 * sm_hal_i2c.c
 *
 *  Created on: Nov 10, 2026
 *      Author: vanlo
 */
#include "sm_hal_i2c.h"

typedef struct {
	I2C_HandleTypeDef   *m_channel;
	uint16_t			m_devaddress;
}ra_i2c_t;
#define _impl(x)         ((ra_i2c_t*)(x))

sm_hal_i2c_t* sm_hal_i2c_init(void* _channel, uint16_t _addr){

	if(!_channel) return NULL;
	ra_i2c_t* _this = (ra_i2c_t*)(malloc(sizeof(ra_i2c_t)));
	_this->m_channel = _channel;
	_this->m_devaddress = _addr;
	return (sm_hal_i2c_t*)_this;
}
void sm_hal_i2c_deinit(sm_hal_i2c_t *_this){

	if(!_this) return;
	free(_this);
}

int32_t sm_hal_i2c_set_addr(sm_hal_i2c_t *_this, uint16_t _addr){

	if(!_this) return -1;
	_impl(_this)->m_devaddress = _addr;
	return 0;
}
int32_t sm_hal_i2c_open(sm_hal_i2c_t *_this){
	if(!_this) return -1;
	HAL_I2C_MspInit(_impl(_this)->m_channel);
	return 0;
}
int32_t sm_hal_i2c_close(sm_hal_i2c_t *_this){
	if(!_this) return -1;
	HAL_I2C_MspDeInit(_impl(_this)->m_channel);
	return 0;
}
int32_t sm_hal_i2c_write(sm_hal_i2c_t *_this, uint16_t mem_address, uint16_t mem_addsize,uint8_t *_data, uint32_t _len, uint32_t time_out){

	if(!_this) return -1;
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(_impl(_this)->m_channel, _impl(_this)->m_devaddress,
			mem_address, mem_addsize, _data, _len, time_out);
	return (ret != HAL_OK) ? -1 : 0;
}
int32_t sm_hal_i2c_read(sm_hal_i2c_t *_this, uint16_t mem_address, uint16_t mem_addsize, uint8_t *_data, uint32_t _len, uint32_t time_out){

	if(!_this) return -1;
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(_impl(_this)->m_channel, _impl(_this)->m_devaddress,
							mem_address, mem_addsize, _data, _len, time_out);
	return (ret != HAL_OK) ? -1 : 0;
}
int32_t sm_hal_i2c_write_read(sm_hal_i2c_t *_this,
                              const uint8_t *_data,
                              uint32_t _data_len,
                              uint8_t* _buf,
                              uint32_t* _recv_len);
