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

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED     = 1000,
  BLINK_SUSPENDED   = 2500,

  BLINK_ALWAYS_ON   = UINT32_MAX,
  BLINK_ALWAYS_OFF  = 0
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

//------------- prototypes -------------//
void led_blinking_task(void);
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
    led_blinking_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
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
    case VENDOR_REQUEST_MICROSOFT:
      if ( request->wIndex == 7 )
      {
        // Get Microsoft OS 2.0 compatible descriptor
        uint16_t total_len;
        memcpy(&total_len, desc_ms_os_20+8, 2);

        return tud_control_xfer(rhport, request, (void*) desc_ms_os_20, total_len);
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
  if ( ! tud_cdc_connected() ) return;

  // connected and there are data available
  if ( ! tud_cdc_available() ) return;

  uint8_t buf[64];

  uint32_t count = tud_cdc_read(buf, sizeof(buf));
  for(uint32_t i=0; i < count; ++i)
  {
#if 0
    tud_cdc_write_char(buf[i]);
    if ( buf[i] == '\r' ) tud_cdc_write_char('\n');
#else
    ;
#endif
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;

  // connected
  if ( dtr && rts )
  {
    // print initial message when connected
    cdc_printf("clk %ld\n", SystemCoreClock);
    tud_cdc_write_flush();
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
