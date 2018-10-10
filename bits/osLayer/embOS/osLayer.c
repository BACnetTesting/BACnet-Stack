/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
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

#include "RTOS.h"
#include "osLayer.h"
#include "CEDebug.h"

void bits_multiTimer_init(
                            volatile TimerControl *timerControl )
{
  timerControl->timeLastReset = bits_sysTimer_get_time () ;
}

uint32_t bits_timer_delta_time(
                              uint32_t prevTime,
                              uint32_t newTime )
{
  // watch for 31-bit wrap (the OS timer is 31 bits in embOS !?
  if ( prevTime <= newTime ) return newTime - prevTime ;
  // we have wrapped
  return newTime + 0x7FFFFFFFu - prevTime + 1u ;
}


inline uint32_t bits_sysTimer_get_time(
                                void )
{
  return (uint32_t) OS_GetTime();
}


uint32_t bits_sysTimer_elapsed_milliseconds(
                                         uint32_t prevTime )
{
  return bits_timer_delta_time ( prevTime, bits_sysTimer_get_time() ) ;
}


uint32_t bits_multiTimer_elapsed_milliseconds(
                                         volatile TimerControl *timerControl )
{
  uint32_t newTime = bits_sysTimer_get_time() ;
  
   return ( bits_timer_delta_time( timerControl->timeLastReset, newTime ) ) ;
}


void bits_multiTimer_reset(
                      volatile TimerControl *timerControl )
{
   timerControl->timeLastReset = bits_sysTimer_get_time();
}

