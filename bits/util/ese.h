/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

// Embedded debug codes for diagnostics. Can be examined by memory browser, 
// or displayed in BACnet Device Description
// or reported by BTA

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MX_ESE  10      

typedef enum {
    // remember to skip all decimal values with embedded '0's
    ese001_01_bcast_pdu_length_too_short=1,
    ese002_02_fwd_pdu_length_too_short_spare,
    ese003_03_incoming_IP_congestion,
    ese004_04_failed_to_malloc,
    ese005_05_tracelog_full,
    ese006_06_incoming_MSTP_congestion,
    ese007_07_bad_malloc_free,
    ese008_08_duplicate_free,
    ese009_09_mstp_output_queue_full,
    ese011_0B_IP_failed_to_send = 11 ,
    ese012_0C_IP_failed_to_alloc,
    ese013_0D_IP_err,
    ese014_0E_Non_BACnetIP_pkt_recd,
    ese015_0F_Unknown_BACnetIP_function,
    ese016_10_Could_Not_Init_IP,
    ese017_11_notValidTodo2,
    ese018_12_incoming_MSTP_race_condition_1,
    ese019_13_incoming_MSTP_race_condition_2,
    ese021_15_incoming_MSTP_race_condition_3 = 21,
    ese022_16_fwd_pdu_length_too_short,
    ese023_17_incoming_MSTP_congestion,
    ese024_18_BTA_buffer_overrun,
    ese025_19_EMM_low_bounds_overrun,
    ese026_1A_EMM_high_bounds_overrun,
    ese027_1B_Corrupted_IP,
    ese028_1C_Attempt_to_send_BTA_before_IP_ready,
    ese029_1D_EMM_trace_table_miss,
    ese031_1F_EMM_spare = 31,
    } ESE;


extern bool sendTokenPassingFrames;
extern uint8_t ltOrUDPoffset, sourceNet, sourceNode;
extern uint8_t eseHead ;
extern uint8_t sys_err_buffer[MX_ESE];
void ese_enqueue_init_fail(ESE rc);

void ese_enqueue_uint8(uint8_t value);
void ese_enqueue(ESE value);
void ese_enqueue_once(ESE value);
bool ese_available(void);
uint8_t ese_dequeue(void);

