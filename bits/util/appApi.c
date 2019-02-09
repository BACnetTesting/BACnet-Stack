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

#include <stdint.h>
#include "appApi.h"
#include "osLayer.h"
#include "BACnetObject.h"
#include "ai.h"
#include "bi.h"
#include "ao.h"
#include "bo.h"
#include "av.h"
#include "bv.h"
#include "device.h"

void Create_Object(const uint devInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, const char *name, const char *description)
{
#if 0
    VirtualDeviceInfo *dev = Find_Virtual_Device(devInstance);
    if (dev == NULL) {
        panicDesc("No device found");
        return;
    }

    BACnetObject *obj = NULL;

    switch (objType) {
    case OBJ_TYPE_AI:
        obj = new AnalogInputObject(objInstance, UNITS_KILOWATTS, name, description);
        dev->pDev->analogInputs.push_back(obj);
        break;
    case OBJ_TYPE_BI:
        obj = new BinaryInputObject(objInstance, name, description);
        dev->pDev->binaryInputs.push_back(obj);
        break;
    case OBJ_TYPE_AO:
        obj = new AnalogOutputObject(objInstance, UNITS_KILOWATTS, name, description);
        dev->pDev->analogOutputs.push_back(obj);
        break;

#if (BACNET_USE_OBJECT_ANALOG_VALUE == 1)
    case OBJ_TYPE_AV:
        obj = new AnalogValueObject(objInstance, UNITS_KILOWATTS, name, description);
        dev->pDev->analogValues.push_back(obj);
        break;
#endif

        obj = new BinaryOutputObject(objInstance, name, description);
        dev->pDev->binaryOutputs.push_back(obj);
        break;

    case OBJ_TYPE_BV:
        obj = new BinaryValueObject(objInstance, name, description);
        dev->pDev->binaryValues.push_back(obj);
        break;
#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
    case OBJ_TYPE_NC:
        obj = new NotificationClass(objInstance, name, description);
        dev->pDev->notificationClasses.push_back(obj);
        break;
#endif
    default:
        panic();
        break;
    }
#endif
}
