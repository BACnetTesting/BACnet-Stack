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
#include "bacenum.h"

#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
#include "ai.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
#include "ao.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_VALUE == 1)
#include "av.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_INPUT == 1)
#include "bi.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )
#include "bo.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_VALUE == 1 )
#include "bv.h"
#endif
#include "device.h"
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
#include "ms-input.h"
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1 )
#include "mso.h"
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
#include "msv.h"
#endif
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "netport.h"
#endif

typedef struct
{
    BACNET_OBJECT_TYPE  bacType;
    IPC_OBJECT_TYPE ipcType;
} BAC_IPC_MAP ;

static BAC_IPC_MAP bacIpcMap[] = { 
    { OBJECT_ANALOG_INPUT, OBJ_TYPE_AI },
    { OBJECT_ANALOG_OUTPUT, OBJ_TYPE_AO },
    { OBJECT_ANALOG_VALUE, OBJ_TYPE_AV },
    { OBJECT_BINARY_INPUT, OBJ_TYPE_BI },
    { OBJECT_BINARY_OUTPUT, OBJ_TYPE_BO },
    { OBJECT_BINARY_VALUE, OBJ_TYPE_BV }
};

BACNET_OBJECT_TYPE BACapi_IPC_to_BACnet_Type( IPC_OBJECT_TYPE ipcObjectType )
{
    for (int i = 0; i < sizeof(bacIpcMap) / sizeof(BAC_IPC_MAP); i++)
    {
        if (bacIpcMap[i].ipcType == ipcObjectType) return bacIpcMap[i].bacType ;
    }
//	switch (ipcObjectType)
//	{
//	case OBJ_TYPE_AI:
//		return OBJECT_ANALOG_INPUT;
//	case OBJ_TYPE_AO:
//		return OBJECT_ANALOG_OUTPUT;
//	case OBJ_TYPE_AV:
//		return OBJECT_ANALOG_VALUE;
//	default:
		panic();
//	}
	return MAX_BACNET_OBJECT_TYPE;
}


IPC_OBJECT_TYPE BACapi_BACnet_to_IPC_Type(BACNET_OBJECT_TYPE bacType)
{
    for (int i = 0; i < sizeof(bacIpcMap) / sizeof(BAC_IPC_MAP); i++)
    {
        if (bacIpcMap[i].bacType == bacType) return bacIpcMap[i].ipcType ;
    }
    panic();
    return OBJ_TYPE_Error ;
}


bool BACapi_isAnalogObject(const IPC_OBJECT_TYPE objType)
{
    if (objType != OBJ_TYPE_AI &&
    	objType != OBJ_TYPE_AO &&
    	objType != OBJ_TYPE_AV) 
    {
        
        return false ;
    }
    return true ;
}

void BACapi_Analog_UpdatePV(const uint32_t deviceInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, float pv)
{
	if ( ! BACapi_isAnalogObject( objType) )
	{
		panic();
		return;
	}

	BACnetObject* currentObject = BACnet_Find_Object(
		deviceInstance,
		BACapi_IPC_to_BACnet_Type(objType),
		objInstance);
	if (currentObject == NULL)
	{
		panic();
		return; 
	}
	currentObject->SetAppPresentValue(pv);
}

void BACapi_Analog_Update_Units(const uint32_t deviceInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, BACAPI_UNITS units)
{
    if (!BACapi_isAnalogObject(objType))
    {
        panic();
        return;
    }

    BACnetAnalogObject* currentObject = (BACnetAnalogObject *) BACnet_Find_Object(
    	deviceInstance,
        BACapi_IPC_to_BACnet_Type(objType),
        objInstance);
    if (currentObject == NULL)
    {
        panic();
        return; 
    }
    
    switch (units)
    {
    case BACAPI_UNITS_DEGREES_CELSIUS: 
        currentObject->Units = UNITS_DEGREES_CELSIUS;
        break;
    default:
        panic();
        // make a decent opaque crossing table
    }
}


void Create_Object(const uint devInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, const char *name, const char *description)
{
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

    case OBJ_TYPE_BO:
        obj = new BinaryOutputObject(objInstance, name, description);
        dev->pDev->binaryOutputs.push_back(obj);
        break;

    case OBJ_TYPE_BV:
        obj = new BinaryValueObject(objInstance, name, description);
        dev->pDev->binaryValues.push_back(obj);
        break;

    case OBJ_TYPE_MSI:
        obj = new MultistateInputObject(objInstance, name, description);
        dev->pDev->multistateInputs.push_back(obj);
        break;

    case OBJ_TYPE_MSO:
        obj = new MultistateOutputObject(objInstance, name, description);
        dev->pDev->multistateOutputs.push_back(obj);
        break;

    case OBJ_TYPE_MSV:
        obj = new MultistateValueObject(objInstance, name, description);
        dev->pDev->multistateValues.push_back(obj);
        break;

#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
    case OBJ_TYPE_NC:
        obj = new NotificationClass(objInstance, name, description);
        dev->pDev->notificationClasses.push_back(obj);
        break;
#endif

    default:
        panicDesc( "Cannot create specified Object Type" );
        break;
    }
}
