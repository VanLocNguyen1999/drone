/*
 * sm_remote.c
 *
 *  Created on: Nov 22, 2026
 *      Author: vanlo
 */

#include "sm_remote.h"
#include "sm_hal_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#define _impl(x) ((remote_impl_t *)(x))

#define FRAME_HEADER_1 0xAA
#define FRAME_HEADER_2 0x55
#define PAYLOAD_LEN 11

typedef enum {
	ST_HEADER_1 = 0,
	ST_HEADER_2,
	ST_LEN,
	ST_PAYLOAD,
	ST_CRC
} remote_state_t;

typedef struct sm_remote {
    sm_hal_uart_t *m_uart;
    uint32_t m_timeout_ms;

    remote_state_t m_state;
    uint8_t m_payload[PAYLOAD_LEN];
    uint8_t m_index;
    uint8_t m_crc;

    sm_remote_ctrl_t m_ctrl;
    uint32_t m_last_rx_tick;
    uint8_t m_has_frame;
    sm_remote_stats_t m_stats;

    sm_remote_frame_fn_t m_frame_cb;
    void *m_frame_arg;
} remote_impl_t;

static remote_impl_t g_remote;

static uint8_t crc8_update(uint8_t crc, uint8_t byte) {
	crc ^= byte;
	for (int i = 0; i < 8; i++) {
		crc = (crc & 0x80) ? (uint8_t) ((crc << 1) ^ 0x07) : (uint8_t) (crc << 1);
	}
	return crc;
}

static void remote_process_frame(remote_impl_t *remote) {

	sm_remote_ctrl_t this;
	/* Little-endian, khop voi ESP32-C3 */
	memcpy(&this.roll, &remote->m_payload[0], 2);
	memcpy(&this.pitch, &remote->m_payload[2], 2);
	memcpy(&this.yaw, &remote->m_payload[4], 2);
	this.throttle = remote->m_payload[6];
	this.sw = remote->m_payload[7] & SM_REMOTE_SW_MASK;
	memcpy(&this.sequence, &remote->m_payload[8], 2);
	this.link = remote->m_payload[10];
	/* Gioi han lai cho an toan */
	if (this.roll > 100) this.roll = 100;
	if (this.roll < -100) this.roll = -100;
	if (this.pitch > 100) this.pitch = 100;
	if (this.pitch < -100) this.pitch = -100;
	if (this.yaw > 100) this.yaw = 100;
	if (this.yaw < -100) this.yaw = -100;
	if (this.throttle > 100) this.throttle = 100;
	remote->m_ctrl = this;
	remote->m_last_rx_tick = HAL_GetTick();
	remote->m_has_frame = 1;
	remote->m_stats.frames_ok++;
	if (remote->m_frame_cb)
		remote->m_frame_cb(&this, remote->m_frame_arg);
}

static void remote_on_byte(uint8_t byte, void *arg) {

	remote_impl_t *this = _impl(arg);
	if (!this)
		return;
	switch (this->m_state) {
		case ST_HEADER_1:
			if (byte == FRAME_HEADER_1)
				this->m_state = ST_HEADER_2;
			break;
		case ST_HEADER_2:
			if (byte == FRAME_HEADER_2)
				this->m_state = ST_LEN;
			else if (byte != FRAME_HEADER_1)
				this->m_state = ST_HEADER_1;
			break;
		case ST_LEN:
			if (byte == PAYLOAD_LEN) {
				this->m_crc = crc8_update(0, byte);
				this->m_index = 0;
				this->m_state = ST_PAYLOAD;
			} else {
				this->m_stats.len_errors++;
				this->m_state = (byte == FRAME_HEADER_1) ? ST_HEADER_2 : ST_HEADER_1;
			}
			break;
		case ST_PAYLOAD:
			this->m_payload[this->m_index++] = byte;
			this->m_crc = crc8_update(this->m_crc, byte);
			if (this->m_index >= PAYLOAD_LEN)
				this->m_state = ST_CRC;
			break;
		case ST_CRC:
			if (byte == this->m_crc)
				remote_process_frame(this);
			else
				this->m_stats.crc_errors++;
			this->m_state = ST_HEADER_1;
			break;
	default:
		this->m_state = ST_HEADER_1;
		break;
	}
}

sm_remote_t* sm_remote_init(sm_hal_uart_t *uart, uint32_t timeout_ms) {

	if (!uart)
		return NULL;

	remote_impl_t *remote = &g_remote;
	if (remote->m_uart == uart)
		return NULL;
	if (remote->m_uart == NULL) {
		memset(remote, 0, sizeof(*remote));
		remote->m_uart = uart;
		remote->m_timeout_ms = timeout_ms ? timeout_ms : SM_REMOTE_TIMEOUT_MS;
		remote->m_state = ST_HEADER_1;
		return (sm_remote_t*) remote;
	}
	return NULL;
}

int32_t sm_remote_start(sm_remote_t *_this) {
	if (!_this)
		return -1;
	remote_impl_t *remote = _impl(_this);
	remote->m_state = ST_HEADER_1;
	sm_hal_uart_set_rx_callback(remote->m_uart, remote_on_byte, remote);
	return sm_hal_uart_start(remote->m_uart);
}

int32_t sm_remote_stop(sm_remote_t *_this) {
	if (!_this)
		return -1;
	return sm_hal_uart_stop(_impl(_this)->m_uart);
}

int32_t sm_remote_deinit(sm_remote_t *_this) {
	if (!_this)
		return -1;
	sm_remote_stop(_this);
	memset(_impl(_this), 0, sizeof(remote_impl_t));
	return 0;
}

int32_t sm_remote_read(sm_remote_t *_this, sm_remote_ctrl_t *out) {
	if (!_this || !out)
		return -1;

	remote_impl_t *remote = _impl(_this);
	sm_remote_ctrl_t ctrl;
	uint8_t has_frame;
	uint32_t last_tick;
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	ctrl = remote->m_ctrl;
	has_frame = remote->m_has_frame;
	last_tick = remote->m_last_rx_tick;
	__set_PRIMASK(primask);

	if (has_frame && ctrl.link == 1
			&& (uint32_t) (HAL_GetTick() - last_tick) < remote->m_timeout_ms) {
		*out = ctrl;
		return 0;
	}
	memset(out, 0, sizeof(*out));
	out->sequence = ctrl.sequence;
	return -1;
}

int32_t sm_remote_get_stats(sm_remote_t *_this, sm_remote_stats_t *out) {
	if (!_this || !out)
		return -1;

	remote_impl_t *remote = _impl(_this);

	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	*out = remote->m_stats;
	__set_PRIMASK(primask);
	return 0;
}

void sm_remote_set_frame_callback(sm_remote_t *_this,
		sm_remote_frame_fn_t cb, void *arg) {
	if (!_this)
		return;
	_impl(_this)->m_frame_cb = cb;
	_impl(_this)->m_frame_arg = arg;
}
