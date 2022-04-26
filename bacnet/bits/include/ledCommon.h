/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

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

#ifndef LEDCOMMON_H
#define LEDCOMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "osLED.h"

//typedef struct {
//	void 			*os_LED_CB ;
//	uint16_t		msPulseTime;
//} LED_CB ;


// todo - push this down a layer
// void bits_board_led_heartbeat_assert ( bool state ) ;

//void bits_led_pulse (
//		LED_CB *ledCB ) ;

//void led_toggle(
//    LED led);

void led_tx_on(
    void);

void led_rx_on(
    void);

//void led_on_interval(
//    LED led,
//    uint16_t interval_ms);

void led_tx_on_interval(
    uint16_t interval_ms);

void led_rx_on_interval(
    uint16_t interval_ms);

void led_rx_error(
    uint16_t interval_ms);

void led_rx_data_frame(
    uint16_t interval_ms);

void led_tx_off(
    void);

void led_rx_off(
    void);

void led_tx_off_delay(
    uint32_t delay_ms);

void led_rx_off_delay(
    uint32_t delay_ms);

void led_tx_toggle(
    void);

void led_rx_toggle(
    void);

bool led_tx_state(
    void);

bool led_rx_state(
    void);

void bits_led_task(
    void);

void led_init(
    void);

#endif
