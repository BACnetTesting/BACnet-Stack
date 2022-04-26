/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

// Embedded debug codes for diagnostics. Can be examined by memory browser,
// or displayed in BACnet Device Description
// or reported by BTA

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BITS_USE_FLASHERR   0

#define MX_ESE 10

typedef enum {
    // remember to skip all decimal values with embedded '0's
    ese001_01_startup=1,
    ese002_02_fwd_pdu_length_too_short_spare,
    ese003_03_incoming_IP_congestion,
    ese004_04_failed_to_malloc,
    ese005_05_tracelog_full,
    ese006_06_incoming_MSTP_congestion,
    ese007_07_bad_malloc_free,
    ese008_08_duplicate_free,
    ese009_09_mstp_output_queue_full,
    ese011_0B_IP_failed_to_send = 11,
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
    ese032_BTA_output_length_exceeded,
    ese033_BTA_output_queue_full,
} ESE;

// extern bool sendTokenPassingFrames;
// extern uint8_t ltOrUDPoffset, sourceNet, sourceNode;
extern uint8_t eseHead;
extern uint8_t sys_err_buffer[MX_ESE];
void ese_enqueue_init_fail(ESE rc);

void ese_enqueue_uint8(uint8_t value);
void ese_enqueue(ESE value);
void ese_enqueue_once(ESE value);
bool ese_available(void);
uint8_t ese_dequeue(void);


#if ( BITS_USE_FLASHERR == 1 )

// todo - move to own header, make independent? (or merge into syserr?)
void FlashErr1msTick(void);
void FlashErr10msTick(void);
void FlashErr_enqueue(uint8_t value);
void FlashErr_enqueue_once(uint8_t value);
void FlashErr_clear(void);

#else

// disable all FlashErr operations

#define FlashErr_enqueue(a)      

#endif
