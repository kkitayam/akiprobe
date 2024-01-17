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

extern usbd_class_driver_t const cmsis_dap;
  
//--------------------------------------------------------------------+
// Application API (Multiple Interfaces)
//--------------------------------------------------------------------+
/**
 * @brief Check if the CMSIS-DAP interface is mounted.
 *
 * This function checks whether the CMSIS-DAP interface specified by the given interface number is mounted or not.
 *
 * @param itf The interface number of the CMSIS-DAP interface.
 * @return true if the CMSIS-DAP interface is mounted, false otherwise.
 */
bool     tud_cmsis_dap_n_mounted         (uint8_t itf);
/**
 * @brief Acquires the request buffer for the specified CMSIS-DAP interface.
 *
 * This function is used to acquire the request buffer for the specified CMSIS-DAP interface.
 *
 * @param itf The interface number.
 * @param pbuf Pointer to a pointer that will be updated with the address of the request buffer.
 *
 * @return The size of the acquired request buffer in bytes.
 */
uint32_t tud_cmsis_dap_n_acquire_request_buffer(uint8_t itf, const uint8_t **pbuf);
/**
 * @brief Releases the request buffer for the CMSIS-DAP interface.
 *
 * This function releases the request buffer for the specified CMSIS-DAP interface.
 *
 * @param itf The interface number.
 */
void     tud_cmsis_dap_n_release_request_buffer(uint8_t itf);
/**
 * @brief Acquires the response buffer for the specified interface.
 *
 * This function is used to acquire the response buffer for the specified interface in the CMSIS-DAP device.
 *
 * @param itf The interface number.
 * @param pbuf Pointer to a pointer that will be updated with the address of the response buffer.
 *
 * @return The size of the acquired response buffer in bytes.
 */
uint32_t tud_cmsis_dap_n_acquire_response_buffer(uint8_t itf, uint8_t **pbuf);
/**
 * @brief Releases the response buffer for the specified CMSIS-DAP interface.
 *
 * This function releases the response buffer used for communication with the CMSIS-DAP interface.
 *
 * @param itf The interface number.
 * @param bufsize The size of the buffer to be released.
 */
void     tud_cmsis_dap_n_release_response_buffer(uint8_t itf, uint32_t bufsize);
   
//--------------------------------------------------------------------+
// Application API (Single Port)
//--------------------------------------------------------------------+
static inline bool     tud_cmsis_dap_mounted         (void);
static inline uint32_t tud_cmsis_dap_acquire_request_buffer(const uint8_t **pbuf);
static inline void     tud_cmsis_dap_release_request_buffer(void);
static inline uint32_t tud_cmsis_dap_acquire_response_buffer(uint8_t **pbuf);
static inline void     tud_cmsis_dap_release_response_buffer(uint32_t bufsize);

//--------------------------------------------------------------------+
// Application Callback API (weak is optional)
//--------------------------------------------------------------------+

// Invoked when received new data
TU_ATTR_WEAK void tud_cmsis_dap_transfer_abort_cb(uint8_t itf);

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

static inline void     tud_cmsis_dap_release_request_buffer(void)
{
  return tud_cmsis_dap_n_release_request_buffer(0);
}

static inline uint32_t tud_cmsis_dap_acquire_response_buffer(uint8_t **pbuf)
{
  return tud_cmsis_dap_n_acquire_response_buffer(0, pbuf);
}

static inline void     tud_cmsis_dap_release_response_buffer(uint32_t bufsize)
{
  return tud_cmsis_dap_n_release_response_buffer(0, bufsize);
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
