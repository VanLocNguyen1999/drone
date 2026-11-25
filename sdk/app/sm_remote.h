/*
 * remote_rx.h
 *
 *  Created on: Nov 22, 2026
 *      Author: vanlo
 */

#ifndef SDK_APP_SM_REMOTE_H_
#define SDK_APP_SM_REMOTE_H_
#include <stdint.h>
#include "sm_hal_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SM_REMOTE_TIMEOUT_MS 200

/* sm_remote_ctrl_t.sw: trang thai nut JOY2_SW (1 = dang nhan, 0 = nha) */
#define SM_REMOTE_SW_PRESSED 1U
#define SM_REMOTE_SW_MASK    0x01U

typedef struct sm_remote sm_remote_t;

typedef struct {
    int16_t roll;      /* -100..100 */
    int16_t pitch;     /* -100..100 */
    int16_t yaw;       /* -100..100 */
    uint8_t throttle;  /* 0..100    */
    uint8_t sw;        /* JOY2_SW: 1 = nhan, 0 = nha */
    uint16_t sequence;
    uint8_t link;
} sm_remote_ctrl_t;

typedef struct {
    uint32_t frames_ok;
    uint32_t crc_errors;
    uint32_t len_errors;
} sm_remote_stats_t;

typedef void (*sm_remote_frame_fn_t)(const sm_remote_ctrl_t *ctrl, void *arg);

sm_remote_t* sm_remote_init(sm_hal_uart_t *uart, uint32_t timeout_ms);
int32_t sm_remote_start(sm_remote_t *_this);
int32_t sm_remote_stop(sm_remote_t *_this);
int32_t sm_remote_deinit(sm_remote_t *_this);
int32_t sm_remote_read(sm_remote_t *_this, sm_remote_ctrl_t *out);
int32_t sm_remote_get_stats(sm_remote_t *_this, sm_remote_stats_t *out);
void sm_remote_set_frame_callback(sm_remote_t *_this,sm_remote_frame_fn_t cb, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* SM_REMOTE_H_ */
