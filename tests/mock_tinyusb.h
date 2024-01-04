/* SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2024 Koji KITAYAMA
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
#ifndef MOCK_TINYUSB_H___
#define MOCK_TINYUSB_H___

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tusb.h"

class MockTinyUsb {
public:
  MOCK_METHOD(bool, usbd_edpt_open, (uint8_t rhport, tusb_desc_endpoint_t const *desc_ep));
  MOCK_METHOD(void, usbd_edpt_close, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(bool, usbd_edpt_xfer, (uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes));
  MOCK_METHOD(bool, usbd_edpt_xfer_fifo, (uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes));
  MOCK_METHOD(bool, usbd_edpt_claim, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(bool, usbd_edpt_release, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(bool, usbd_edpt_busy, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(void, usbd_edpt_stall, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(void, usbd_edpt_clear_stall, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(bool, usbd_edpt_stalled, (uint8_t rhport, uint8_t ep_addr));
  MOCK_METHOD(void, usbd_sof_enable, (uint8_t rhport, bool en));
  MOCK_METHOD(bool, usbd_open_edpt_pair, (uint8_t rhport, uint8_t const* p_desc, uint8_t ep_count, uint8_t xfer_type, uint8_t* ep_out, uint8_t* ep_in));
};

void mtu_set_instance(MockTinyUsb* p);

#endif
