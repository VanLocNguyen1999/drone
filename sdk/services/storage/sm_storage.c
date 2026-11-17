/*
 * sm_storage.c
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#include "sm_storage.h"
#include "sm_hal_flash.h"
#include "crc_calc.h"

#define _impl(x) ((sm_storage_config_t *)(x))

#define MAGIC_NUMBER_LENGTH       4U
#define CRC_LENGTH                2U
#define HEADER_CONFIG_DATA_SIZE   (MAGIC_NUMBER_LENGTH + CRC_LENGTH)

static const uint32_t MAGIC_NUMBER = 0x5A5A5A5AU;

static int8_t sm_storage_init(sm_storage_t *_this);
static int8_t sm_storage_load(sm_storage_t *_this, void *_config);
static int8_t sm_storage_store(sm_storage_t *_this, const void *_config);
static int8_t sm_storage_clear(sm_storage_t *_this);
static int8_t sm_storage_free(sm_storage_t *_this);

static sm_storage_proc_t sm_storage_proc_default = {
    .init  = sm_storage_init,
    .load  = sm_storage_load,
    .store = sm_storage_store,
    .clear = sm_storage_clear,
    .free  = sm_storage_free
};

/* Check configuration storage range */
static int32_t sm_storage_check(sm_storage_t *_this) {
	if (!_this || !_impl(_this)->m_flash)
		return -1;

	uint32_t addr = _impl(_this)->m_start_address;
	uint32_t config_size = _impl(_this)->m_config_size;

	int32_t block_size = sm_hal_flash_get_block_size(_impl(_this)->m_flash);
	int32_t lowest_addr = sm_hal_flash_get_lowest_addr(_impl(_this)->m_flash);
	if (block_size <= 0 || config_size == 0 || addr < lowest_addr)
		return -1;

	uint32_t offset = addr - lowest_addr;

	if (offset >= (uint32_t) block_size || HEADER_CONFIG_DATA_SIZE > (uint32_t) block_size - offset)
		return -1;

	if (config_size > (uint32_t) block_size - offset - HEADER_CONFIG_DATA_SIZE)
		return -1;
	return 0;
}

sm_storage_config_t* sm_storage_create(sm_hal_flash_t* _flash, uint32_t _start_address, uint32_t _config_size) {

	sm_storage_config_t *_this = (sm_storage_config_t*) malloc(sizeof(sm_storage_config_t));
	if (!_this || _flash)
		return NULL;

	memset(_this, 0, sizeof(*_this));
	_this->m_flash = _flash;
	_this->m_start_address = _start_address;
	_this->m_config_size = _config_size;
	_this->m_base.m_proc = &sm_storage_proc_default;

	if (sm_storage_check((sm_storage_t*) _this) < 0) {
		free(_this);
		return NULL;
	}

	return _this;
}

static int8_t sm_storage_init(sm_storage_t *_this) {
	return sm_storage_check(_this) == 0 ? 0 : -1;
}
static int8_t sm_storage_free(sm_storage_t *_this) {
	if (!_this)
		return -1;

	free(_this);
	return 0;
}

static int8_t sm_storage_clear(sm_storage_t *_this) {
	if (sm_storage_check(_this) < 0)
		return -1;

	uint32_t addr = _impl(_this)->m_start_address;
	uint32_t magic = 0;
	if (sm_hal_flash_write_block(_impl(_this)->m_flash, addr, &magic, sizeof(magic)) < 0)
		return -1;
	return 0;
}

static int8_t sm_storage_store(sm_storage_t *_this, const void *_config) {
	if (!_config || sm_storage_check(_this) < 0)
		return -1;

	uint32_t addr = _impl(_this)->m_start_address;
	uint32_t config_size = _impl(_this)->m_config_size;
	uint32_t buff_size = HEADER_CONFIG_DATA_SIZE + config_size;

	uint8_t header[HEADER_CONFIG_DATA_SIZE];
	if (sm_hal_flash_read(_impl(_this)->m_flash, addr, header, sizeof(header)) < 0)
		return -1;
	uint16_t crc = crc_calculate_16bit((uint8_t*) _config, config_size);
	uint16_t crc_old = 0;
	memcpy(&crc_old, &header[MAGIC_NUMBER_LENGTH],CRC_LENGTH);

	if (memcmp(header, &MAGIC_NUMBER, MAGIC_NUMBER_LENGTH) == 0 && crc_old == crc) {
		const uint8_t *old_data = (const uint8_t*) (uintptr_t) (addr + HEADER_CONFIG_DATA_SIZE);
		if (memcmp(old_data, _config, config_size) == 0)
			return 2;
	}

	uint8_t config_data[buff_size];
	memcpy(config_data, &MAGIC_NUMBER,MAGIC_NUMBER_LENGTH);
	memcpy(&config_data[MAGIC_NUMBER_LENGTH], &crc,CRC_LENGTH);
	memcpy(&config_data[HEADER_CONFIG_DATA_SIZE], _config, config_size);
	/* HAL Flash handles read/modify/erase/write */
	if (sm_hal_flash_write_block(_impl(_this)->m_flash, addr, config_data, buff_size) < 0)
		return -1;
	return 0;
}

/* Load configuration */
static int8_t sm_storage_load(sm_storage_t *_this, void *_config) {
	if (!_config || sm_storage_check(_this) < 0)
		return -1;

	uint32_t addr = _impl(_this)->m_start_address;
	uint32_t config_size = _impl(_this)->m_config_size;

	uint8_t header[HEADER_CONFIG_DATA_SIZE];

	if (sm_hal_flash_read(_impl(_this)->m_flash, addr, header, sizeof(header)) < 0)
		return -1;

	/* Check MAGIC */
	if (memcmp(header, &MAGIC_NUMBER,MAGIC_NUMBER_LENGTH) != 0)
		return -1;
	uint16_t crc_load = 0;
	memcpy(&crc_load, &header[MAGIC_NUMBER_LENGTH],CRC_LENGTH);

	const uint8_t *flash_data = (const uint8_t*) (uintptr_t) (addr + HEADER_CONFIG_DATA_SIZE);
	uint16_t crc_calc = crc_calculate_16bit((uint8_t*) flash_data, config_size);
	if (crc_load != crc_calc)
		return -2;
	memcpy(_config, flash_data, config_size);
	return 0;
}
