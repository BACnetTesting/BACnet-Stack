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

#pragma once

#ifdef __GNUC__
#include <math.h>       // For NAN
#endif
#include "config.h"

#include "bacenum.h"
#include "stdint.h"
#include <math.h>
#include "BACnetObject.h"

#define RELINQUISH_DEFAULT_ANALOG   0.0

#if 0
class BACnetAnalogObject : public BACnetObject
{
public:

    BACnetAnalogObject(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetObject(nInstance, name, description),
        Present_Value(0.0f)
    {}

    float Present_Value;
    BACNET_ENGINEERING_UNITS Units;

#if (INTRINSIC_REPORTING == 1)
    BACNET_EVENT_STATE Event_State;
    uint32_t Time_Delay;
    uint32_t Notification_Class;
    float High_Limit;
    float Low_Limit;
    float Deadband;
    unsigned Limit_Enable:2;
    unsigned Event_Enable:3;
    BACNET_NOTIFY_TYPE Notify_Type; // unsigned Notify_Type:1;
    ACKED_INFO Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
    BACNET_DATE_TIME Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
    /* time to generate event notification */
    uint32_t Remaining_Time_Delay;
    /* AckNotification informations */
    ACK_NOTIFICATION Ack_notify_data;
#endif

};
#endif

#ifdef _MSC_VER
// this pragma disables (under microsoft, YMMV) the 'new behavior' regarding initializing arrays to 0 by default
#pragma warning( disable : 4351 )
#endif

#if 0
class BACnetCommandableAnalogObject : public BACnetAnalogObject
{
public:
    BACnetCommandableAnalogObject(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetAnalogObject(nInstance, name, description),
        prioritySet ( ),                                   // zero-initialize arrays in C++
        priorityValues(),
        relinquish_default3(NAN)
    {}

    bool prioritySet[BACNET_MAX_PRIORITY];
    float priorityValues[BACNET_MAX_PRIORITY];

    float relinquish_default3;

    void SweepToPresentValue(void);
    // bool IsDominantPriority(unsigned priority);


};
#endif

