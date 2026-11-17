/*
 * stm_hal_uart.h
 *
 *  Created on: Nov 16, 2026
 *      Author: vanlo
 */

#ifndef SDK_HAL_SM_HAL_UART_H_
#define SDK_HAL_SM_HAL_UART_H_

#include "sm_types.h"

typedef void sm_hal_uart_t;
typedef void (*sm_hal_uart_rx_irq_fn_t)(uint8_t, void*);
typedef void (*sm_hal_uart_tx_irq_fn_t)(void*);
sm_hal_uart_t* sm_hal_uart_init_ex(void *channel, uint32_t baud,uint8_t stop_bit,
		uint8_t data_bit, uint32_t irq_type);
int32_t sm_hal_uart_deinit(sm_hal_uart_t *_this);

int32_t sm_hal_uart_start(sm_hal_uart_t *_this);
int32_t sm_hal_uart_stop(sm_hal_uart_t *_this);

int32_t sm_hal_uart_set_baudrate(sm_hal_uart_t *_this, uint32_t baudrate);
int32_t sm_hal_uart_write(sm_hal_uart_t *_this, uint8_t *data, uint32_t len) ;
int32_t sm_hal_uart_read(sm_hal_uart_t* _this, uint8_t* data, uint32_t len);

void sm_hal_uart_set_rx_callback(sm_hal_uart_t *_this,sm_hal_uart_rx_irq_fn_t cb, void *arg);
void sm_hal_uart_set_tx_callback(sm_hal_uart_t *_this,sm_hal_uart_tx_irq_fn_t cb, void *arg);
#endif /* SDK_HAL_SM_HAL_UART_H_ */
