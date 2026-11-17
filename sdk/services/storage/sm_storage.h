/*
 * sm_storage.h
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_SERVICES_STORAGE_SM_STORAGE_H_
#define SDK_SERVICES_STORAGE_SM_STORAGE_H_
#include "sm_hal_flash.h"

typedef struct sm_storage sm_storage_t ;
typedef struct sm_storage_config sm_storage_config_t ;
typedef struct {

	int8_t (*init)(sm_storage_t*);
	int8_t (*load)(sm_storage_t*, void*);
	int8_t (*store)(sm_storage_t*, const void*);
	int8_t (*clear)(sm_storage_t*);
	int8_t (*free)(sm_storage_t*);
} sm_storage_proc_t;

struct sm_storage {
	const sm_storage_proc_t* m_proc;
};

struct sm_storage_config{

	sm_hal_flash_t* m_flash;
	sm_storage_t m_base;
    uint32_t m_start_address;
    volatile uint32_t m_config_size;
};

sm_storage_config_t* sm_storage_create(sm_hal_flash_t* _flash, uint32_t _start_address, uint32_t _config_size);
#endif /* SDK_SERVICES_STORAGE_SM_STORAGE_H_ */
