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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vendor_device.h"
#include "board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "DAP_config.h"
#include "DAP.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

//#define DEBUG
#define URL  "studio.keil.arm.com/auth/login/"

const tusb_desc_webusb_url_t desc_url =
{
  .bLength         = 3 + sizeof(URL) - 1,
  .bDescriptorType = 3, // WEBUSB URL type
  .bScheme         = 1, // 0: http, 1: https
  .url             = URL
};

//------------- prototypes -------------//
void cdc_task(void);
void dap_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  DAP_Setup();

  while (1)
  {
    tud_task(); // tinyusb device task
    cdc_task();
    dap_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// CMSIS DAP v2 use vendor class
//--------------------------------------------------------------------+

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
  // nothing to with DATA & ACK stage
  if (stage != CONTROL_STAGE_SETUP ) return true;

  switch (request->bRequest)
  {
    case VENDOR_REQUEST_WEBUSB:
     // match vendor request in BOS descriptor
          // Get landing page url
          return tud_control_xfer(rhport, request, (void*)&desc_url, desc_url.bLength);

    case VENDOR_REQUEST_MICROSOFT:
      if ( request->wIndex == 7 )
      {
        // Get Microsoft OS 2.0 compatible descriptor
        uint16_t total_len;
        memcpy(&total_len, desc_ms_os_20+8, 2);

        return tud_control_xfer(rhport, request, (void*)desc_ms_os_20, total_len);
      }else
      {
        return false;
      }

    default:
      // stall unknown request
      return false;
  }

  return true;
}

void tud_vendor_transfer_abort_cb(uint8_t itf)
{
  DAP_TransferAbort = 1;
}


#include <stdarg.h>
void cdc_printf(const char* str, ...) __attribute__ ((format (printf, 1, 2)));
void cdc_printf(const char* str, ...)
{
  static char buf[64];
  va_list args;
  va_start(args, str);
  vsprintf(buf, str, args);
  va_end(args);
  tud_cdc_write_str(buf);
}

void dap_task(void)
{
  const uint8_t *p_req;
  uint8_t *p_rsp;
  unsigned sz_req = tud_vendor_acquire_request_buffer(&p_req);
  if (!sz_req) return;
  unsigned sz_rsp = tud_vendor_acquire_response_buffer(&p_rsp);
  TU_ASSERT(sz_rsp,);

  uint32_t result = DAP_ExecuteCommand(p_req, p_rsp);
#ifdef DEBUG
  cdc_printf("%x %x -> %lx %x\n", p_req[0], sz_req, result, p_rsp[1]);
#endif

  tud_vendor_release_request_buffer();
  tud_vendor_release_response_buffer(result & 0xFFFFU);

#ifdef DEBUG
  tud_cdc_write_flush();
#endif
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
#ifndef DEBUG
  static uint8_t tx_buf[64];
  static unsigned tx_length;
  static unsigned tx_index;
  static uint8_t rx_buf[64];
  static unsigned rx_length;
  static unsigned rx_index;

  if ( ! tud_cdc_connected() ) {
    // clear FIFO
    board_uart_read(rx_buf, sizeof(rx_buf));
    tx_index = 0;
    tx_length = 0;
    rx_index = 0;
    rx_length = 0;
    return;
  }

  if (rx_index == rx_length) {
    rx_index = 0;
    rx_length = board_uart_read(rx_buf, sizeof(rx_buf));
  }
  if (rx_index < rx_length) {
    rx_index += tud_cdc_write(&rx_buf[rx_index], rx_length - rx_index);
    tud_cdc_write_flush();
  }

  if ((tx_index == tx_length) && tud_cdc_available()) {
    tx_index = 0;
    tx_length = tud_cdc_read(tx_buf, sizeof(tx_buf));
  }
  if (tx_index < tx_length)
    tx_index += board_uart_write(&tx_buf[tx_index], tx_length - tx_index);
#else
  if ( ! tud_cdc_connected() ) return;
  if (tud_cdc_available()) {
    tud_cdc_read_flush();
  }
#endif
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding)
{
  board_uart_set_baudrate(line_coding->bit_rate);
}
