/*
 * stm_hal_uart.c
 *
 *  Created on: Nov 16, 2026
 *      Author: vanlo
 */

#include "sm_hal_uart.h"
#include "usart.h"
#include <string.h>

#define _impl(x) ((stm_uart_t *)(x))
#define MAX_UART_CHANNEL 4

typedef struct stm_uart {
    UART_HandleTypeDef *m_uart;

    uint32_t m_baud;
    uint8_t m_stop_bit;
    uint8_t m_data_bit;
    uint32_t m_irq_type;

    volatile uint8_t m_tx_completed;
    uint8_t m_rx_byte;

    sm_hal_uart_rx_irq_fn_t m_rx_cb;
    void *m_rx_arg;

    sm_hal_uart_tx_irq_fn_t m_tx_cb;
    void *m_tx_arg;
} stm_uart_t;

static stm_uart_t g_uart[MAX_UART_CHANNEL];

sm_hal_uart_t* sm_hal_uart_init_ex(void *channel, uint32_t baud,uint8_t stop_bit,
		uint8_t data_bit, uint32_t irq_type) {

	if (!channel)
		return NULL;
	for (int i = 0; i < MAX_UART_CHANNEL; i++) {
		stm_uart_t *uart = &g_uart[i];
		if (uart->m_uart == channel)
			return NULL;
		if (uart->m_uart == NULL) {
			memset(uart, 0, sizeof(*uart));
			uart->m_uart = (UART_HandleTypeDef*) channel;
			uart->m_baud = baud;
			uart->m_stop_bit = stop_bit;
			uart->m_data_bit = data_bit;
			uart->m_irq_type = irq_type;
			uart->m_tx_completed = 1;
			return (sm_hal_uart_t*) uart;
		}
	}
	return NULL;
}

int32_t sm_hal_uart_start(sm_hal_uart_t *_this) {
	if (!_this)
		return -1;
	HAL_NVIC_EnableIRQ((IRQn_Type) _impl(_this)->m_irq_type);
	if (HAL_UART_Receive_IT(_impl(_this)->m_uart, &_impl(_this)->m_rx_byte, 1) != HAL_OK) {
		return -1;
	}

	return 0;
}

int32_t sm_hal_uart_stop(sm_hal_uart_t *_this) {
	if (!_this)
		return -1;

	stm_uart_t *uart = _impl(_this);
	HAL_UART_Abort(uart->m_uart);
	HAL_NVIC_DisableIRQ((IRQn_Type) uart->m_irq_type);
	return 0;
}

int32_t sm_hal_uart_deinit(sm_hal_uart_t *_this) {
	if (!_this)
		return -1;
	sm_hal_uart_stop(_this);
	memset(_impl(_this), 0, sizeof(stm_uart_t));
	return 0;
}

int32_t sm_hal_uart_set_baudrate(sm_hal_uart_t *_this, uint32_t baudrate) {
	if (!_this)
		return -1;
	_impl(_this)->m_uart->Init.BaudRate = baudrate;
	if (HAL_UART_Init(_impl(_this)->m_uart) != HAL_OK)
		return -1;
	_impl(_this)->m_baud = baudrate;
	return 0;
}

int32_t sm_hal_uart_write(sm_hal_uart_t *_this, uint8_t *data, uint32_t len) {
	if (!_this || !data || !len || len > 65535)
		return -1;
	if (!_impl(_this)->m_tx_completed)
		return -1;
	_impl(_this)->m_tx_completed = 0;
	if (HAL_UART_Transmit_IT(_impl(_this)->m_uart, data, (uint16_t) len) != HAL_OK) {
		_impl(_this)->m_tx_completed = 1;
		return -1;
	}
	return 0;
}
int32_t sm_hal_uart_read(sm_hal_uart_t* _this, uint8_t* data, uint32_t len){
    if (!_this){
        return -1;
    }
    if(HAL_UART_Receive_IT(_impl(_this)->m_uart, data, len) != HAL_OK){

    	return -1;
    }
    return 0;
}
void sm_hal_uart_set_rx_callback(sm_hal_uart_t *_this,sm_hal_uart_rx_irq_fn_t cb, void *arg) {

	if (!_this)
		return;
	_impl(_this)->m_rx_cb = cb;
	_impl(_this)->m_rx_arg = arg;
}

void sm_hal_uart_set_tx_callback(sm_hal_uart_t *_this,
		sm_hal_uart_tx_irq_fn_t cb, void *arg) {
	if (!_this)
		return;
	_impl(_this)->m_tx_cb = cb;
	_impl(_this)->m_tx_arg = arg;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	for (int i = 0; i < MAX_UART_CHANNEL; i++) {
		stm_uart_t *uart = &g_uart[i];
		if (uart->m_uart == huart) {
			uart->m_tx_completed = 1;
			if (uart->m_tx_cb)uart->m_tx_cb(uart->m_tx_arg);
			break;
		}
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	for (int i = 0; i < MAX_UART_CHANNEL; i++) {
		stm_uart_t *uart = &g_uart[i];
		if (uart->m_uart == huart) {
			uint8_t data = uart->m_rx_byte;
			HAL_UART_Receive_IT(uart->m_uart, &uart->m_rx_byte, 1);
			if (uart->m_rx_cb){
				uart->m_rx_cb(data, uart->m_rx_arg);
			}
			break;
		}
	}
}
