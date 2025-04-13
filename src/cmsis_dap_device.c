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

#include "tusb_option.h"

#if (CFG_TUD_ENABLED && CFG_TUD_CMSIS_DAP)

#include "device/usbd.h"
#include "device/usbd_pvt.h"

#include "cmsis_dap_device.h"

#include "DAP_config.h"
#include "DAP.h"

extern uint32_t SWO_GetTransportMode(void);

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
typedef struct
{
  uint8_t itf_num;
  uint8_t ep_in;
  uint8_t ep_out;
  #if (SWO_STREAM != 0)
  uint8_t ep_swo;
  #endif

  volatile uint8_t request_wp;
  volatile uint8_t request_rp;
  volatile uint8_t response_wp;
  volatile uint8_t response_rp;

  /*------------- From this point, data is not cleared by bus reset -------------*/
  uint16_t epout_sz[DAP_PACKET_COUNT];
  uint16_t epin_sz[DAP_PACKET_COUNT];
  #if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
  tu_fifo_t swo_ff;
  uint8_t swo_ff_buf[SWO_BUFFER_SIZE];
  #endif

  // Endpoint Transfer buffer
  CFG_TUSB_MEM_ALIGN uint8_t epout_buf[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
  CFG_TUSB_MEM_ALIGN uint8_t epin_buf[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
  #if (SWO_STREAM != 0)
  CFG_TUSB_MEM_ALIGN uint8_t epswo_buf[DAP_PACKET_SIZE];
  #endif
} cmsis_dap_interface_t;

CFG_TUSB_MEM_SECTION static cmsis_dap_interface_t _cmsis_dap_itf[CFG_TUD_CMSIS_DAP];

#define ITF_MEM_RESET_SIZE   offsetof(cmsis_dap_interface_t, epout_sz)

//--------------------------------------------------------------------+
// Weak stubs: invoked if no strong implementation is available
//--------------------------------------------------------------------+
TU_ATTR_WEAK void tud_cmsis_dap_transfer_abort_cb(uint8_t intf) {
  (void) intf;
}

TU_ATTR_WEAK void tud_cmsis_dap_swo_write_cb(uint8_t intf) {
  (void) intf;
}

//--------------------------------------------------------------------+
// APPLICATION API
//--------------------------------------------------------------------+
bool tud_cmsis_dap_n_mounted (uint8_t itf)
{
  return _cmsis_dap_itf[itf].ep_in && _cmsis_dap_itf[itf].ep_out;
}

//--------------------------------------------------------------------+
// Read API
//--------------------------------------------------------------------+
static void _prep_out_transaction(cmsis_dap_interface_t* p_itf)
{
  uint8_t const rhport = 0;

  // claim endpoint
  TU_VERIFY(usbd_edpt_claim(rhport, p_itf->ep_out), );

  uint8_t wp = p_itf->request_wp;

  uint8_t occupancy = wp - p_itf->request_rp;
  if (occupancy < DAP_PACKET_COUNT) {
    unsigned idx = wp % DAP_PACKET_COUNT;
    usbd_edpt_xfer(rhport, p_itf->ep_out, p_itf->epout_buf[idx], DAP_PACKET_SIZE);
  } else {
    usbd_edpt_release(rhport, p_itf->ep_out);
  }
}

uint32_t tud_cmsis_dap_n_acquire_request_buffer(uint8_t itf, const uint8_t **pbuf)
{
  TU_ASSERT(pbuf, 0);
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];

  uint8_t rp = p_itf->request_rp;
  uint8_t wp = p_itf->request_wp;
  if (wp == rp)
    return 0;

  unsigned idx = rp % DAP_PACKET_COUNT;
  unsigned n = idx;
  // Check if the last packet was received.
  while ((wp != rp) &&
         (p_itf->epout_buf[n][0] == ID_DAP_QueueCommands)) {
    ++rp;
    ++n;
    if (n == DAP_PACKET_COUNT)
      n = 0U;
  }
  if (wp == rp) // not received
    return 0;

  // Replace consecutive QueueCommands to ExecuteCommands
  rp = p_itf->request_rp;
  n = idx;
  while ((wp != rp) &&
         (p_itf->epout_buf[n][0] == ID_DAP_QueueCommands)) {
    p_itf->epout_buf[n][0] = ID_DAP_ExecuteCommands;
    ++rp;
    ++n;
    if (n == DAP_PACKET_COUNT)
      n = 0U;
  }
  *pbuf = p_itf->epout_buf[idx];
  return p_itf->epout_sz[idx];
}

void tud_cmsis_dap_n_release_request_buffer(uint8_t itf)
{
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  ++p_itf->request_rp;
  _prep_out_transaction(p_itf);
}

//--------------------------------------------------------------------+
// Write API
//--------------------------------------------------------------------+
static void maybe_transmit(cmsis_dap_interface_t* p_itf)
{
  uint8_t const rhport = 0;

  // skip if previous transfer not complete
  TU_VERIFY( usbd_edpt_claim(rhport, p_itf->ep_in), );

  uint8_t rp = p_itf->response_rp;

  if (p_itf->response_wp != rp) {
    unsigned idx = rp % DAP_PACKET_COUNT;
    usbd_edpt_xfer(rhport, p_itf->ep_in, p_itf->epin_buf[idx], p_itf->epin_sz[idx]);
  } else {
    usbd_edpt_release(rhport, p_itf->ep_in);
  }
}

uint32_t tud_cmsis_dap_n_acquire_response_buffer(uint8_t itf, uint8_t **pbuf)
{
  TU_ASSERT(pbuf, 0);
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];

  uint8_t occupancy = p_itf->response_wp - p_itf->response_rp;
  if (occupancy < DAP_PACKET_COUNT) {
    unsigned idx = p_itf->response_wp % DAP_PACKET_COUNT;
    *pbuf = p_itf->epin_buf[idx];
    return DAP_PACKET_SIZE;
  }
  return 0;
}

void tud_cmsis_dap_n_release_response_buffer(uint8_t itf, uint32_t bufsize)
{
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  unsigned idx = p_itf->response_wp % DAP_PACKET_COUNT;
  p_itf->epin_sz[idx] = bufsize;
  ++p_itf->response_wp;
  maybe_transmit(p_itf);
}

#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
//--------------------------------------------------------------------+
// SWO API
//--------------------------------------------------------------------+
#if (SWO_STREAM != 0)
static void maybe_transmit_swo(cmsis_dap_interface_t* p_itf)
{
  if (2 != SWO_GetTransportMode()) return;

  const uint8_t rhport = 0;

  // Claim the endpoint
  TU_VERIFY(usbd_edpt_claim(rhport, p_itf->ep_swo), );

  // Pull data from FIFO
  const uint16_t count = tu_fifo_read_n(&p_itf->swo_ff, p_itf->epswo_buf, DAP_PACKET_SIZE);

  if (count) {
    TU_ASSERT(usbd_edpt_xfer(rhport, p_itf->ep_swo, p_itf->epswo_buf, count), );
  } else {
    // Release endpoint since we don't make any transfer
    // Note: data is dropped if terminal is not connected
    usbd_edpt_release(rhport, p_itf->ep_swo);
  }
}
#else
# define maybe_transmit_swo(p_itf)
#endif

uint32_t tud_cmsis_dap_n_swo_enqueue(uint8_t itf, void const *data, uint32_t len)
{
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  uint16_t ret = tu_fifo_write_n(&p_itf->swo_ff, data, (uint16_t)TU_MIN(len, UINT16_MAX));
  maybe_transmit_swo(p_itf);
  return ret;
}

uint32_t tud_cmsis_dap_n_swo_dequeue(uint8_t itf, void* buffer, uint32_t bufsize)
{
  if (1 != SWO_GetTransportMode()) return 0;

  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  const uint16_t count = tu_fifo_read_n(&p_itf->swo_ff, buffer, bufsize);
  return count;
}

uint32_t tud_cmsis_dap_n_swo_free(uint8_t itf)
{
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  return tu_fifo_remaining(&p_itf->swo_ff);
}

uint32_t tud_cmsis_dap_n_swo_used(uint8_t itf)
{
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  return tu_fifo_count(&p_itf->swo_ff);
}

uint32_t tud_cmsis_dap_n_swo_clear(uint8_t itf)
{
  cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[itf];
  return tu_fifo_clear(&p_itf->swo_ff);
}

#endif

//--------------------------------------------------------------------+
// USBD Driver API
//--------------------------------------------------------------------+
void cmsis_dapd_init(void)
{
  tu_memclr(_cmsis_dap_itf, sizeof(_cmsis_dap_itf));
#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
  for(uint_fast8_t i = 0; i < CFG_TUD_CMSIS_DAP; i++) {
    cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[i];
    tu_fifo_config(&p_itf->swo_ff, p_itf->swo_ff_buf, TU_ARRAY_SIZE(p_itf->swo_ff_buf), 1, true);
  }
#endif
}

bool cmsis_dapd_deinit(void)
{
  return true;
}

void cmsis_dapd_reset(uint8_t rhport)
{
  (void) rhport;

  for(uint8_t i=0; i<CFG_TUD_CMSIS_DAP; i++)
  {
    cmsis_dap_interface_t* p_itf = &_cmsis_dap_itf[i];

    tu_memclr(p_itf, ITF_MEM_RESET_SIZE);
#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))
    tu_fifo_clear(&p_itf->swo_ff);
    tu_fifo_set_overwritable(&p_itf->swo_ff, true);
#endif
  }
}

uint16_t cmsis_dapd_open(uint8_t rhport, tusb_desc_interface_t const * desc_itf, uint16_t max_len)
{
  TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == desc_itf->bInterfaceClass, 0);

  uint8_t const * p_desc = tu_desc_next(desc_itf);
  uint8_t const * desc_end = p_desc + max_len;

  // Find available interface
  cmsis_dap_interface_t* p_cmsis_dap = NULL;
  for(uint8_t i=0; i<CFG_TUD_CMSIS_DAP; i++)
  {
    if ( _cmsis_dap_itf[i].ep_in == 0 && _cmsis_dap_itf[i].ep_out == 0 )
    {
      p_cmsis_dap = &_cmsis_dap_itf[i];
      break;
    }
  }
  TU_VERIFY(p_cmsis_dap, 0);

  p_cmsis_dap->itf_num = desc_itf->bInterfaceNumber;
  if (desc_itf->bNumEndpoints)
  {
    // skip non-endpoint descriptors
    while ( (TUSB_DESC_ENDPOINT != tu_desc_type(p_desc)) && (p_desc < desc_end) )
    {
      p_desc = tu_desc_next(p_desc);
    }

    // Open endpoint pair with usbd helper
    TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, 2, TUSB_XFER_BULK, &p_cmsis_dap->ep_out, &p_cmsis_dap->ep_in), 0);

    p_desc += 2 * sizeof(tusb_desc_endpoint_t);

  #if (SWO_STREAM != 0)
    if (desc_itf->bNumEndpoints == 3)
    {
      TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, 1, TUSB_XFER_BULK, NULL, &p_cmsis_dap->ep_swo), 0);
      p_desc += sizeof(tusb_desc_endpoint_t);
    }
  #endif

    // Prepare for incoming data
    if ( p_cmsis_dap->ep_out )
    {
      TU_ASSERT(usbd_edpt_xfer(rhport, p_cmsis_dap->ep_out, p_cmsis_dap->epout_buf[0], sizeof(p_cmsis_dap->epout_buf)), 0);
    }

    if ( p_cmsis_dap->ep_in ) maybe_transmit(p_cmsis_dap);
  }

  return (uint16_t) ((uintptr_t) p_desc - (uintptr_t) desc_itf);
}

bool cmsis_dapd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
  (void) rhport;
  (void) result;

  uint8_t itf = 0;
  cmsis_dap_interface_t* p_itf = _cmsis_dap_itf;

  for ( ; ; itf++, p_itf++)
  {
    if (itf >= TU_ARRAY_SIZE(_cmsis_dap_itf)) return false;

    if ( ( ep_addr == p_itf->ep_out ) || ( ep_addr == p_itf->ep_in )
#if (SWO_STREAM != 0)
         || ( ep_addr == p_itf->ep_swo )
#endif
      )
      break;
  }

  if ( ep_addr == p_itf->ep_out )
  {
    if (xferred_bytes) {
      unsigned idx = p_itf->request_wp % DAP_PACKET_COUNT;
      p_itf->epout_sz[idx] = xferred_bytes;
      if (ID_DAP_TransferAbort == p_itf->epout_buf[idx][0]) {
        tud_cmsis_dap_transfer_abort_cb(itf);
      } else {
        ++p_itf->request_wp;
      }
    }
    _prep_out_transaction(p_itf);
  }
  else if ( ep_addr == p_itf->ep_in )
  {
    ++p_itf->response_rp;
    // Send complete, try to send more if possible
    maybe_transmit(p_itf);
  }
#if (SWO_STREAM != 0)
  else if ( ep_addr == p_itf->ep_swo )
  {
    // try to send more if possible
    maybe_transmit_swo(p_itf);
  }
#endif

  return true;
}

#endif
