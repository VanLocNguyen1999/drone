/*
 * sm_hal_flash.h
 *
 *  Created on: Nov 16, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_FLASH_H_
#define SDK_HAL_SM_HAL_FLASH_H_
#include "sm_types.h"

typedef void sm_hal_flash_t;

sm_hal_flash_t* sm_hal_flash_init(void *handle);
void sm_hal_flash_deinit(sm_hal_flash_t *_this);

int32_t sm_hal_flash_get_block_size(sm_hal_flash_t *_this) ;
int32_t sm_hal_flash_get_lowest_addr(sm_hal_flash_t *_this) ;

int32_t sm_hal_flash_write_block(sm_hal_flash_t *_this, uint32_t _addr, void *_data, int32_t _size);
int32_t sm_hal_flash_read(sm_hal_flash_t *_this, uint32_t _addr, void *_data,int32_t _size);
int32_t sm_hal_flash_write_page(sm_hal_flash_t *_this, uint32_t addr,void *data, int32_t size);

#endif /* SDK_HAL_SM_HAL_FLASH_H_ */
