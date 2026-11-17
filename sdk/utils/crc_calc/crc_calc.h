/*
 * crc_calc.h
 *
 *  Created on: Nov 17, 2026
 *      Author: vanlo
 */

#ifndef SDK_UTILS_CRC_CALC_CRC_CALC_H_
#define SDK_UTILS_CRC_CALC_CRC_CALC_H_
#include "sm_types.h"
/* ---------- Defines for 8-bit SAE J1850 CRC calculation (Not reflected) ------------------------------------------------------- */
#define CRC_8_RESULT_WIDTH                  8u
#define CRC_8_POLYNOMIAL                    0x1Du
#define CRC_8_INIT_VALUE                    0xFFu
#define CRC_8_XOR_VALUE                     0xFFu
#define CRC_8_MODE							RUNTTIME

/* ---------- Defines for 16-bit CCITT CRC calculation (Not reflected) ---------------------------------------------------------- */
#define CRC_16_RESULT_WIDTH                 16u
#define CRC_16_POLYNOMIAL                   0x1021u
#define CRC_16_INIT_VALUE                   0xFFFFu
#define CRC_16_XOR_VALUE                    0x0000u
#define CRC_16_MODE							TABLE

/* ---------- Defines for 32-bit CCITT CRC calculation (Reflected) -------------------------------------------------------------- */
#define CRC_32_RESULT_WIDTH                 32u
#define CRC_32_POLYNOMIAL                   0x04C11DB7u
#define CRC_32_INIT_VALUE                   0xFFFFFFFFu
#define CRC_32_XOR_VALUE                    0xFFFFFFFFu
#define CRC_32_MODE							RUNTTIME



uint8_t crc_calculate_8bit(const uint8_t *_buffer, uint16_t length);
uint16_t crc_calculate_16bit(const uint8_t *_buffer, uint32_t length);
uint32_t crc_calculate_32bit(const uint8_t *_buffer, uint32_t length);
#endif /* SDK_UTILS_CRC_CALC_CRC_CALC_H_ */
