/*
 * sm_hal_flash.c
 *
 *  Created on: Nov 16, 2026
 *      Author: vanlo
 */

#include "sm_hal_flash.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

#define _impl(x) ((sm_flash_impl_t *)(x))

#define FLASH_SECTOR_ADDR   0x08020000U
#define FLASH_SECTOR_SIZE   (128U * 1024U)
#define FLASH_BUFFER_SIZE   2048U

typedef struct sm_flash_impl {
    uint32_t m_page_size;
    uint32_t m_lowest_addr;
    uint32_t m_highest_addr;
} sm_flash_impl_t;

static sm_flash_impl_t g_flash;

static int32_t flash_check(uint32_t addr, int32_t size) {
	if (size <= 0 || addr < FLASH_SECTOR_ADDR)
		return -1;

	uint32_t offset = addr - FLASH_SECTOR_ADDR;

	if (offset >= FLASH_BUFFER_SIZE
			|| (uint32_t) size > FLASH_BUFFER_SIZE - offset)
		return -1;

	return 0;
}

static void flash_reset_cache(void) {
	if (FLASH->ACR & FLASH_ACR_DCEN) {
		__HAL_FLASH_DATA_CACHE_DISABLE();
		__HAL_FLASH_DATA_CACHE_RESET();
		__HAL_FLASH_DATA_CACHE_ENABLE();
	}
}

sm_hal_flash_t* sm_hal_flash_init(void *handle) {
	(void) handle;

	g_flash.m_page_size = FLASH_BUFFER_SIZE;
	g_flash.m_lowest_addr = FLASH_SECTOR_ADDR;
	g_flash.m_highest_addr = FLASH_SECTOR_ADDR + FLASH_BUFFER_SIZE - 1U;
	return (sm_hal_flash_t*) &g_flash;
}

void sm_hal_flash_deinit(sm_hal_flash_t *_this) {
	(void) _this;
}

int32_t sm_hal_flash_get_block_size(sm_hal_flash_t *_this) {
	if (!_this)
		return -1;

	return FLASH_BUFFER_SIZE;
}
int32_t sm_hal_flash_get_lowest_addr(sm_hal_flash_t *_this) {

	if (!_this)
		return FLASH_SECTOR_ADDR;

	return _impl(_this)->m_lowest_addr;
}
int32_t sm_hal_flash_write_block(sm_hal_flash_t *_this, uint32_t _addr,
		void *_data, int32_t _size) {
	if (!_this || !_data || flash_check(_addr, _size) < 0)
		return -1;

	uint32_t buffer[FLASH_BUFFER_SIZE / 4U];
	uint32_t offset = _addr - FLASH_SECTOR_ADDR;
	memcpy(buffer, (const void*) (uintptr_t) FLASH_SECTOR_ADDR,FLASH_BUFFER_SIZE);
	memcpy((uint8_t*) buffer + offset, _data, _size);

	if (HAL_FLASH_Unlock() != HAL_OK)
		return -2;

	FLASH_EraseInitTypeDef erase = { 0 };
	uint32_t sector_error = 0xFFFFFFFFU;

	erase.TypeErase = FLASH_TYPEERASE_SECTORS;
	erase.Sector = FLASH_SECTOR_5;
	erase.NbSectors = 1;
	erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK|| sector_error != 0xFFFFFFFFU) {
		HAL_FLASH_Lock();
		return -3;
	}

	flash_reset_cache();

	int32_t result = 0;
	for (uint32_t i = 0; i < FLASH_BUFFER_SIZE / 4U; i++) {
		if (buffer[i] == 0xFFFFFFFFU)
			continue;
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_SECTOR_ADDR + i * 4U, buffer[i]) != HAL_OK) {
			result = -4;
			break;
		}
	}

	if (HAL_FLASH_Lock() != HAL_OK && result == 0)
		result = -5;

	flash_reset_cache();

	if (result == 0 && memcmp((const void*) (uintptr_t) FLASH_SECTOR_ADDR, buffer, FLASH_BUFFER_SIZE) != 0){
		result = -6;
	}
	return result;
}
int32_t sm_hal_flash_read(sm_hal_flash_t *_this, uint32_t _addr, void *_data,
		int32_t _size) {
	if (!_this || !_data || flash_check(_addr, _size) < 0)
		return -1;
	memcpy(_data, (const void*) (uintptr_t) _addr, (size_t) _size);
	return 0;
}

int32_t sm_hal_flash_write_page(sm_hal_flash_t *_this, uint32_t addr,void *data, int32_t size) {
	return sm_hal_flash_write_block(_this, addr, data, size);
}
