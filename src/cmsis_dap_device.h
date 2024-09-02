/* SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2022 Koji KITAYAMA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. */

#ifndef _TUSB_CMSIS_DAP_DEVICE_H_
#define _TUSB_CMSIS_DAP_DEVICE_H_

#include "common/tusb_common.h"
#include "device/usbd_pvt.h"

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// Application API (Multiple Interfaces)
//--------------------------------------------------------------------+
bool     tud_cmsis_dap_n_mounted         (uint8_t itf);
uint32_t tud_cmsis_dap_n_acquire_request_buffer(uint8_t itf, const uint8_t **pbuf);
void     tud_cmsis_dap_n_release_request_buffer(uint8_t itf);
uint32_t tud_cmsis_dap_n_acquire_response_buffer(uint8_t itf, uint8_t **pbuf);
void     tud_cmsis_dap_n_release_response_buffer(uint8_t itf, uint32_t bufsize);
bool     tud_cmsis_dap_n_swo_write(uint8_t itf, void const *data, uint16_t len);

//--------------------------------------------------------------------+
// Application API (Single Port)
//--------------------------------------------------------------------+
static inline bool     tud_cmsis_dap_mounted         (void);
static inline uint32_t tud_cmsis_dap_acquire_request_buffer(const uint8_t **pbuf);
static inline void     tud_cmsis_dap_release_request_buffer(void);
static inline uint32_t tud_cmsis_dap_acquire_response_buffer(uint8_t **pbuf);
static inline void     tud_cmsis_dap_release_response_buffer(uint32_t bufsize);
static inline bool     tud_cmsis_dap_swo_write(void const *data, uint16_t len);

//--------------------------------------------------------------------+
// Application Callback API (weak is optional)
//--------------------------------------------------------------------+

// Invoked when abort request is received
void tud_cmsis_dap_transfer_abort_cb(uint8_t itf);

// Invoked when SWO transfer finished
void tud_cmsis_dap_swo_write_cb(uint8_t intf);
//--------------------------------------------------------------------+
// Inline Functions
//--------------------------------------------------------------------+

static inline bool tud_cmsis_dap_mounted (void)
{
  return tud_cmsis_dap_n_mounted(0);
}

static inline uint32_t tud_cmsis_dap_acquire_request_buffer(const uint8_t **pbuf)
{
  return tud_cmsis_dap_n_acquire_request_buffer(0, pbuf);
}

static inline void tud_cmsis_dap_release_request_buffer(void)
{
  tud_cmsis_dap_n_release_request_buffer(0);
}

static inline uint32_t tud_cmsis_dap_acquire_response_buffer(uint8_t **pbuf)
{
  return tud_cmsis_dap_n_acquire_response_buffer(0, pbuf);
}

static inline void tud_cmsis_dap_release_response_buffer(uint32_t bufsize)
{
  tud_cmsis_dap_n_release_response_buffer(0, bufsize);
}

static inline bool tud_cmsis_dap_swo_write(void const *data, uint16_t len)
{
  return tud_cmsis_dap_n_swo_write(0, data, len);
}

//--------------------------------------------------------------------+
// Internal Class Driver API
//--------------------------------------------------------------------+
void     cmsis_dapd_init(void);
void     cmsis_dapd_reset(uint8_t rhport);
uint16_t cmsis_dapd_open(uint8_t rhport, tusb_desc_interface_t const * itf_desc, uint16_t max_len);
bool     cmsis_dapd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes);

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CMSIS_DAP_DEVICE_H_ */
