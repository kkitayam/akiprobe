/*
 * Copyright (c) 2013-2021 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        29. March 2021
 * $Revision:    V2.0.1
 *
 * Project:      CMSIS-DAP Source
 * Title:        SWO.c CMSIS-DAP SWO I/O
 *
 *---------------------------------------------------------------------------*/

#include "DAP_config.h"
#include "DAP.h"
#include <stdint.h>

#if (SWO_STREAM != 0)
#ifdef DAP_FW_V1
#error "SWO Streaming Trace not supported in DAP V1!"
#endif
#endif

#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))


#define SWO_STREAM_TIMEOUT      50U     /* Stream timeout in ms */

#define USB_BLOCK_SIZE          512U    /* USB Block Size */

// Trace State
static uint8_t  TraceTransport =  0U;       /* Trace Transport */
static uint8_t  TraceMode      =  0U;       /* Trace Mode */
static uint8_t  TraceStatus    =  0U;       /* Trace Status without Errors */
static uint8_t  TraceError[2]  = {0U, 0U};  /* Trace Error flags (banked) */
static uint8_t  TraceError_n   =  0U;       /* Active Trace Error bank */

#if (TIMESTAMP_CLOCK != 0U)
// Trace Timestamp
static volatile struct {
  uint32_t index;
  uint32_t tick;
} TraceTimestamp;
#endif

// Trace Helper functions
static void     ClearTrace     (void);
static uint32_t GetTraceCount  (void);
uint8_t  GetTraceStatus (void);
#if 0
static void     SetTraceError  (uint8_t flag);
#endif
// Trace Buffer functions
uint32_t TraceBuffer_IsUpdated(void);
uint32_t TraceBuffer_GetCount(void);
void     TraceBuffer_Clear (void);
void     TraceBuffer_Drain (uint8_t *data, uint32_t num);

#if (SWO_MANCHESTER != 0)

// Enable or disable SWO Mode (Manchester)
//   enable: enable flag
//   return: 1 - Success, 0 - Error
__WEAK uint32_t SWO_Mode_Manchester (uint32_t enable) {
  return (0U);
}

// Configure SWO Baudrate (Manchester)
//   baudrate: requested baudrate
//   return:   actual baudrate or 0 when not configured
__WEAK uint32_t SWO_Baudrate_Manchester (uint32_t baudrate) {
  return (0U);
}

// Control SWO Capture (Manchester)
//   active: active flag
//   return: 1 - Success, 0 - Error
__WEAK uint32_t SWO_Control_Manchester (uint32_t active) {
  return (0U);
}

// Start SWO Capture (Manchester)
//   buf: pointer to buffer for capturing
//   num: number of bytes to capture
__WEAK void SWO_Capture_Manchester (uint8_t *buf, uint32_t num) {
}

// Get SWO Pending Trace Count (Manchester)
//   return: number of pending trace data bytes
__WEAK uint32_t SWO_GetCount_Manchester (void) {
}

#endif  /* (SWO_MANCHESTER != 0) */


// Clear Trace Errors and Data
static void ClearTrace (void)
{
  TraceError[0] = 0U;
  TraceError[1] = 0U;
  TraceError_n  = 0U;
  TraceBuffer_Clear();

#if (TIMESTAMP_CLOCK != 0U)
  TraceTimestamp.index = 0U;
  TraceTimestamp.tick  = 0U;
#endif
}

// Get Trace Count
//   return: number of available data bytes in trace buffer
uint32_t GetTraceCount (void)
{
  return TraceBuffer_GetCount();
}

// Get Trace Status (clear Error flags)
//   return: Trace Status (Active flag and Error flags)
uint8_t GetTraceStatus (void)
{
  uint8_t  status;
  uint32_t n;

  n = TraceError_n;
  TraceError_n ^= 1U;
  status = TraceStatus | TraceError[n];
  TraceError[n] = 0U;

  return (status);
}

#if 0
// Set Trace Error flag(s)
//   flag:  error flag(s) to set
static void SetTraceError (uint8_t flag) {
  TraceError[TraceError_n] |= flag;
}
#endif


// Process SWO Transport command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Transport (const uint8_t *request, uint8_t *response) {
  uint8_t  transport;
  uint32_t result;

  if ((TraceStatus & DAP_SWO_CAPTURE_ACTIVE) == 0U) {
    transport = *request;
    switch (transport) {
      case 0U:
      case 1U:
#if (SWO_STREAM != 0)
      case 2U:
#endif
        TraceTransport = transport;
        result = 1U;
        break;
      default:
        result = 0U;
        break;
    }
  } else {
    result = 0U;
  }

  if (result != 0U) {
    *response = DAP_OK;
  } else {
    *response = DAP_ERROR;
  }

  return ((1U << 16) | 1U);
}


// Process SWO Mode command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Mode (const uint8_t *request, uint8_t *response) {
  uint8_t  mode;
  uint32_t result;

  mode = *request;

  switch (TraceMode) {
#if (SWO_UART != 0)
    case DAP_SWO_UART:
      SWO_Mode_UART(0U);
      break;
#endif
#if (SWO_MANCHESTER != 0)
    case DAP_SWO_MANCHESTER:
      SWO_Mode_Manchester(0U);
      break;
#endif
    default:
      break;
  }

  switch (mode) {
    case DAP_SWO_OFF:
      result = 1U;
      break;
#if (SWO_UART != 0)
    case DAP_SWO_UART:
      result = SWO_Mode_UART(1U);
      break;
#endif
#if (SWO_MANCHESTER != 0)
    case DAP_SWO_MANCHESTER:
      result = SWO_Mode_Manchester(1U);
      break;
#endif
    default:
      result = 0U;
      break;
  }
  if (result != 0U) {
    TraceMode = mode;
  } else {
    TraceMode = DAP_SWO_OFF;
  }

  TraceStatus = 0U;

  if (result != 0U) {
    *response = DAP_OK;
  } else {
    *response = DAP_ERROR;
  }

  return ((1U << 16) | 1U);
}


// Process SWO Baudrate command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Baudrate (const uint8_t *request, uint8_t *response) {
  uint32_t baudrate;

  baudrate = (uint32_t)(*(request+0) <<  0) |
             (uint32_t)(*(request+1) <<  8) |
             (uint32_t)(*(request+2) << 16) |
             (uint32_t)(*(request+3) << 24);

  switch (TraceMode) {
#if (SWO_UART != 0)
    case DAP_SWO_UART:
      baudrate = SWO_Baudrate_UART(baudrate);
      break;
#endif
#if (SWO_MANCHESTER != 0)
    case DAP_SWO_MANCHESTER:
      baudrate = SWO_Baudrate_Manchester(baudrate);
      break;
#endif
    default:
      baudrate = 0U;
      break;
  }

  if (baudrate == 0U) {
    TraceStatus = 0U;
  }

  *response++ = (uint8_t)(baudrate >>  0);
  *response++ = (uint8_t)(baudrate >>  8);
  *response++ = (uint8_t)(baudrate >> 16);
  *response   = (uint8_t)(baudrate >> 24);

  return ((4U << 16) | 4U);
}


// Process SWO Control command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Control (const uint8_t *request, uint8_t *response)
{
  uint8_t  active;
  uint32_t result;

  active = *request & DAP_SWO_CAPTURE_ACTIVE;

  if (active != (TraceStatus & DAP_SWO_CAPTURE_ACTIVE)) {
    if (active) {
      ClearTrace();
    }
    switch (TraceMode) {
#if (SWO_UART != 0)
      case DAP_SWO_UART:
        result = SWO_Control_UART(active);
        break;
#endif
#if (SWO_MANCHESTER != 0)
      case DAP_SWO_MANCHESTER:
        result = SWO_Control_Manchester(active);
        break;
#endif
      default:
        result = 0U;
        break;
    }
    if (result != 0U) {
      TraceStatus = active;
    }
  } else {
    result = 1U;
  }

  if (result != 0U) {
    *response = DAP_OK;
  } else {
    *response = DAP_ERROR;
  }

  return ((1U << 16) | 1U);
}


// Process SWO Status command and prepare response
//   response: pointer to response data
//   return:   number of bytes in response
uint32_t SWO_Status (uint8_t *response) {
  uint8_t  status;
  uint32_t count;

  status = GetTraceStatus();
  count  = GetTraceCount();

  *response++ = status;
  *response++ = (uint8_t)(count >>  0);
  *response++ = (uint8_t)(count >>  8);
  *response++ = (uint8_t)(count >> 16);
  *response   = (uint8_t)(count >> 24);

  return (5U);
}


// Process SWO Extended Status command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_ExtendedStatus (const uint8_t *request, uint8_t *response) {
  uint8_t  cmd;
  uint8_t  status;
  uint32_t count;
#if (TIMESTAMP_CLOCK != 0U)
  uint32_t index;
  uint32_t tick;
#endif
  uint32_t num;

  num = 0U;
  cmd = *request;

  if (cmd & 0x01U) {
    status = GetTraceStatus();
    *response++ = status;
    num += 1U;
  }

  if (cmd & 0x02U) {
    count = GetTraceCount();
    *response++ = (uint8_t)(count >>  0);
    *response++ = (uint8_t)(count >>  8);
    *response++ = (uint8_t)(count >> 16);
    *response++ = (uint8_t)(count >> 24);
    num += 4U;
  }

#if (TIMESTAMP_CLOCK != 0U)
  if (cmd & 0x04U) {
    do {
      index = TraceTimestamp.index;
      tick  = TraceTimestamp.tick;
    } while (TraceBuffer_IsUpdated() != 0U);
    *response++ = (uint8_t)(index >>  0);
    *response++ = (uint8_t)(index >>  8);
    *response++ = (uint8_t)(index >> 16);
    *response++ = (uint8_t)(index >> 24);
    *response++ = (uint8_t)(tick  >>  0);
    *response++ = (uint8_t)(tick  >>  8);
    *response++ = (uint8_t)(tick  >> 16);
    *response++ = (uint8_t)(tick  >> 24);
    num += 4U;
  }
#endif

  return ((1U << 16) | num);
}


// Process SWO Data command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Data (const uint8_t *request, uint8_t *response) {
  uint8_t  status;
  uint32_t count;
  uint32_t n;

  status = GetTraceStatus();
  count  = GetTraceCount();

  if (TraceTransport == 1U) {
    n = (uint32_t)(*(request+0) << 0) |
        (uint32_t)(*(request+1) << 8);
    if (n > (DAP_PACKET_SIZE - 4U)) {
      n = DAP_PACKET_SIZE - 4U;
    }
    if (count > n) {
      count = n;
    }
  } else {
    count = 0U;
  }

  *response++ = status;
  *response++ = (uint8_t)(count >> 0);
  *response++ = (uint8_t)(count >> 8);

  if (TraceTransport == 1U) {
    TraceBuffer_Drain(response, count);
  }

  return ((2U << 16) | (3U + count));
}

uint32_t SWO_GetTraceMode(void)
{
  return TraceMode;
}

uint32_t SWO_GetTransportMode(void)
{
  return TraceTransport;
}

#endif  /* ((SWO_UART != 0) || (SWO_MANCHESTER != 0)) */
