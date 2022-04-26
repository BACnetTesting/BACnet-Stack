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

#include "configProj.h"

#include "appApi.h"
#include "osLayer.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"
#include "BACnetObject.h"
#include "bacnet/bacenum.h"
#include "bacnet/bactext.h"
#include "eLib/util/eLibDebug.h"
#include "bacnet/bacenum.h"
#include "bacnet/basic/object/device.h"
#include "eLib/util/emm.h"
#include "bacnet/bacstr.h"
#include "eLib/util/llist.h"
#include "bacnet/bits/util/BACnetToString.h"

#ifdef BAC_DEBUG
#include "assert.h"
#endif

#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
#include "bacnet/basic/object/ai.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
#include "bacnet/basic/object/ao.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_VALUE == 1)
#include "bacnet/basic/object/av.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_INPUT == 1)
#include "bacnet/basic/object/bi.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )
#include "bacnet/basic/object/bo.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_VALUE == 1 )
#include "bacnet/basic/object/bv.h"
#endif
#if ( BACNET_USE_OBJECT_CALENDAR == 1 )
#include "bacnet/basic/object/calendar.h"
#endif
#if (BACNET_USE_OBJECT_CHARACTER_STRING_VALUE == 1)
#include "bacnet/basic/object/csv.h"
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
#include "bacnet/basic/object/ms-input.h"
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1 )
#include "bacnet/basic/object/mso.h"
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
#include "bacnet/basic/object/msv.h"
#endif
#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
#include "bacnet/basic/object/schedule.h"
#endif
#if (BACNET_USE_OBJECT_TRENDLOG == 1)
#include "bacnet/basic/object/trendlog.h"
#endif
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "bacnet/basic/object/netport.h"
#endif
#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
#include "bacnet/basic/object/nc.h"
#endif
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "bacnet/basic/object/netport.h"
#endif

//typedef struct
//{
//    BACNET_OBJECT_TYPE  bacType;
//    IPC_OBJECT_TYPE ipcType;
//} BAC_IPC_MAP ;
//
//static BAC_IPC_MAP bacIpcMap[] = { 
//    { OBJECT_ANALOG_INPUT, OBJ_TYPE_AI },
//    { OBJECT_ANALOG_OUTPUT, OBJ_TYPE_AO },
//    { OBJECT_ANALOG_VALUE, OBJ_TYPE_AV },
//    { OBJECT_BINARY_INPUT, OBJ_TYPE_BI },
//    { OBJECT_BINARY_OUTPUT, OBJ_TYPE_BO },
//    { OBJECT_BINARY_VALUE, OBJ_TYPE_BV },
//    { OBJECT_MULTISTATE_INPUT, OBJ_TYPE_MSI },
//    { OBJECT_MULTISTATE_OUTPUT, OBJ_TYPE_MSO },
//    { OBJECT_MULTISTATE_VALUE, OBJ_TYPE_MSV }
//};
//
//BACNET_OBJECT_TYPE BACapi_IPC_to_BACnet_Type( IPC_OBJECT_TYPE ipcObjectType )
//{
//    for (int i = 0; i < sizeof(bacIpcMap) / sizeof(BAC_IPC_MAP); i++)
//    {
//        if (bacIpcMap[i].ipcType == ipcObjectType) return bacIpcMap[i].bacType ;
//    }
//    panic();
//	return MAX_BACNET_OBJECT_TYPE;
//}
//
//
//IPC_OBJECT_TYPE BACapi_BACnet_to_IPC_Type(BACNET_OBJECT_TYPE bacType)
//{
//    for (int i = 0; i < sizeof(bacIpcMap) / sizeof(BAC_IPC_MAP); i++)
//    {
//        if (bacIpcMap[i].bacType == bacType) return bacIpcMap[i].ipcType ;
//    }
//    panic();
//    return OBJ_TYPE_Error ;
//}


static bool DummyServerSideWriteOutputFn(const BACNET_OBJECT * currentObject, const int priority, const BACNET_APPLICATION_DATA_VALUE* value)
{
    char tstring[400];
    BACNET_OBJECT_PROPERTY_VALUE topv;
    topv.value = (BACNET_APPLICATION_DATA_VALUE *) value;
    bacapp_snprintf_value(tstring, sizeof(tstring), &topv);

    const char *oname = BACnet_ObjectID_ToString(currentObject->objectType, currentObject->objectInstance);

#ifdef AUTOCREATE_SITE
    // pDev2 is null - to be fixed...
    printf("Writing: Dev:%d.%s=%s,  pri[%d]\r\n", 9, /*currentObject->pDev2->bacObj.objectInstance,*/ oname, tstring, priority);
#endif

    return true;
}

static void DummyStackUpdate(BACNET_OBJECT* currentObject )
{
}

/*
    Data operations.    Many TBD

    Server-side Write:  e.g. BACnet Client (Workstation) writes to our BACnet stack
    Server-side Read    e.g. BACnet Client (Workstation) reads from our BACnet stack
    Client-side Write   our BACnet stack writes to another box
    Client-side Read    our BACNet stack reads from another box
    Stack Write         our BACnet stack writes to our application (or hardware)
    Stack Update        our BACnet stack reads from our application (or hardware) (may be a null operation)
    Application Write   our Application writes to our BACnet stack
    Application Read    our Application reads from our BACnet stack

*/

bool (*bits_ServerSide_Write_Output_Fn) (const BACNET_OBJECT * currentObject, const int priority, const BACNET_APPLICATION_DATA_VALUE* value) = DummyServerSideWriteOutputFn;
// 2021-11-09 I decided to place this functionality in the object
// void (*bits_Stack_Update) (BACNET_OBJECT* currentObject) = DummyStackUpdate;

// todo 1 - this is now going to have to be per datalink - writing to a IAP device is going to be different to writing to the Application Entity, for example.
void BACapi_Server_Side_Write_Output_Init(
    bool (*Update_Function_Write_Output) (const BACNET_OBJECT * currentObject, const int priority, const BACNET_APPLICATION_DATA_VALUE* value))
{
    bits_ServerSide_Write_Output_Fn = Update_Function_Write_Output;
}


bool BACapi_isAnalogObject(const BACNET_OBJECT_TYPE objType)
{
    if (objType != OBJECT_ANALOG_INPUT &&
        objType != OBJECT_ANALOG_OUTPUT &&
        objType != OBJECT_ANALOG_VALUE)
    {
        return false;
    }
    return true;
}


bool BACapi_isBinaryObject(const BACNET_OBJECT_TYPE objType)
{
    if (objType != OBJECT_BINARY_INPUT &&
        objType != OBJECT_BINARY_OUTPUT &&
        objType != OBJECT_BINARY_VALUE)
    {
        return false;
    }
    return true;
}


bool BACapi_isMultistateObject(const BACNET_OBJECT_TYPE objType)
{
    if (objType != OBJECT_MULTISTATE_INPUT &&
        objType != OBJECT_MULTISTATE_OUTPUT &&
        objType != OBJECT_MULTISTATE_VALUE)
    {
        return false;
    }
    return true;
}


extern ROUTER_PORT* headRouterPort;


LLIST_HDR *Find_Descriptor_List(
    const DEVICE_OBJECT_DATA* pDev, 
    const BACNET_OBJECT_TYPE objType )
{
    switch (objType)
    {
    case OBJECT_ANALOG_INPUT:
        return (LLIST_HDR * ) &pDev->AI_Descriptor_List;
    case OBJECT_ANALOG_OUTPUT:
        return (LLIST_HDR *)&pDev->AO_Descriptor_List;
    case OBJECT_ANALOG_VALUE:
        return (LLIST_HDR *)&pDev->AV_Descriptor_List;
    case OBJECT_BINARY_INPUT:
        return (LLIST_HDR *)&pDev->BI_Descriptor_List;
    case OBJECT_BINARY_OUTPUT:
        return (LLIST_HDR *)&pDev->BO_Descriptor_List;
    case OBJECT_BINARY_VALUE:
        return (LLIST_HDR *)&pDev->BV_Descriptor_List;
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
    case OBJECT_MULTISTATE_INPUT:
        return (LLIST_HDR *)&pDev->MSI_Descriptor_List;
    case OBJECT_MULTISTATE_OUTPUT:
        return (LLIST_HDR *)&pDev->MSO_Descriptor_List;
    case OBJECT_MULTISTATE_VALUE:
        return (LLIST_HDR *)&pDev->MSV_Descriptor_List;
#endif
    case OBJECT_SCHEDULE:
        return (LLIST_HDR *)&pDev->Schedule_Descriptor_List;
    case OBJECT_CALENDAR:
        return (LLIST_HDR *)&pDev->Calendar_Descriptor_List;
    default:
        panic();
        break;
    }
    return NULL;
}


BACNET_OBJECT* BACnet_Find_Object_from_DevPtr(
    const DEVICE_OBJECT_DATA* pDev,
    const BACNET_OBJECT_TYPE objType,
    const uint32_t objInstance)
{
    LLIST_HDR *llhdr = Find_Descriptor_List(pDev, objType);
    if (!llhdr) return NULL ;

    return Generic_Instance_To_Object(llhdr, objInstance);
}


BACNET_OBJECT* BACnet_Find_Object(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint32_t objInstance)
{
    DEVICE_OBJECT_DATA* pDev = Device_Find_Device(deviceInstance);
    if (pDev == NULL)
    {
        panic();
        return NULL;
    }

    //VirtualDeviceInfo* vdev = Device_Find_VirtualDevice(deviceInstance);
    //if (vdev == NULL)
    //{
    //    ClientSideDevCB* csdCB = Device_Find_Device(deviceInstance);
    //    if (csdCB == NULL)
    //    {
    //        dbMessage(DBD_Application, DB_ERROR, "appAPI: Could not find Device [Dev:%07u]", deviceInstance);
    //        return NULL;
    //    }
    //    pDev = csdCB->pDev;
    //    if (pDev == NULL)
    //    {
    //        panic();
    //        return NULL;
    //    }
    //}
    //else
    //{
    //    pDev = vdev->pDev;
    //    if (pDev == NULL)
    //    {
    //        panic();
    //        return NULL;
    //    }
    //}

    return BACnet_Find_Object_from_DevPtr(pDev, objType, objInstance);
}


// Useful for AI, AO, AV
bool BACapi_Get_PV_double(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    double* pv)
{
    double rvalue;

    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);

    if (currentObject == NULL)
    {
        dbMessage(DBD_Application, DB_ERROR, "appAPI: Could not find BACnet Object   [%s]", BACnet_ObjectID_ToString(objType, objInstance));
        return false;
    }

    switch (objType)
    {
#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1 )
    case OBJECT_ANALOG_INPUT:
        rvalue = ((ANALOG_INPUT_DESCR*)currentObject)->analogCommon.Present_Value;
        break;
#endif

#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    case OBJECT_ANALOG_OUTPUT:
        rvalue = ((ANALOG_OUTPUT_DESCR*)currentObject)->analogCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_ANALOG_VALUE == 1 )
    case OBJECT_ANALOG_VALUE:
        rvalue = ((ANALOG_VALUE_DESCR*)currentObject)->analogCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_BINARY_INPUT == 1 )
    case OBJECT_BINARY_INPUT:
        rvalue = ((BINARY_INPUT_DESCR*)currentObject)->binaryCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )
    case OBJECT_BINARY_OUTPUT:
        rvalue = ((BINARY_OUTPUT_DESCR*)currentObject)->binaryCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_BINARY_VALUE == 1 )
    case OBJECT_BINARY_VALUE:
        rvalue = ((BINARY_VALUE_DESCR*)currentObject)->binaryCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
    case OBJECT_MULTISTATE_INPUT:
        rvalue = ((MULTISTATE_INPUT_DESCR*)currentObject)->multistateCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1 )
    case OBJECT_MULTISTATE_OUTPUT:
        rvalue = ((MULTISTATE_OUTPUT_DESCR*)currentObject)->multistateCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
    case OBJECT_MULTISTATE_VALUE:
        rvalue = ((MULTISTATE_VALUE_DESCR*)currentObject)->multistateCommon.Present_Value;
        break;
#endif
    default:
        panic();
        return false;
    }

    *pv = rvalue;
    return true;
}


// FOR BI, BO, BV
bool BACapi_Get_PV_boolean(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    bool* pv)
{
    bool rvalue;

    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);

    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    switch (objType)
    {
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    case OBJECT_ANALOG_INPUT:
        rvalue = ((ANALOG_INPUT_DESCR*)currentObject)->analogCommon.Present_Value ? true : false;
        break;
#endif
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    case OBJECT_ANALOG_OUTPUT:
        rvalue = ((ANALOG_OUTPUT_DESCR*)currentObject)->analogCommon.Present_Value ? true : false;
        break;
    case OBJECT_ANALOG_VALUE:
        rvalue = ((ANALOG_VALUE_DESCR*)currentObject)->analogCommon.Present_Value ? true : false;
        break;
    case OBJECT_BINARY_INPUT:
        rvalue = ((BINARY_INPUT_DESCR*)currentObject)->binaryCommon.Present_Value ? true : false;
        break;
    case OBJECT_BINARY_OUTPUT:
        rvalue = ((BINARY_OUTPUT_DESCR*)currentObject)->binaryCommon.Present_Value ? true : false;
        break;
    case OBJECT_BINARY_VALUE:
        rvalue = ((BINARY_VALUE_DESCR*)currentObject)->binaryCommon.Present_Value ? true : false;
        break;
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
    case OBJECT_MULTISTATE_INPUT:
        rvalue = ((MULTISTATE_INPUT_DESCR*)currentObject)->multistateCommon.Present_Value ? true : false;
        break;
    case OBJECT_MULTISTATE_OUTPUT:
        rvalue = ((MULTISTATE_OUTPUT_DESCR*)currentObject)->multistateCommon.Present_Value ? true : false;
        break;
    case OBJECT_MULTISTATE_VALUE:
        rvalue = ((MULTISTATE_VALUE_DESCR*)currentObject)->multistateCommon.Present_Value ? true : false;
        break;
#endif
    default:
        panic();
        return false;
    }

    *pv = rvalue;
    return true;
}


// FOR Multi-State Input, (MSI, MSO, MSV)
bool BACapi_Get_PV_uint(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    unsigned int* pv)
{
    unsigned int rvalue;

    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);

    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    switch (objType)
    {
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    case OBJECT_ANALOG_INPUT:
        rvalue = (unsigned int)((ANALOG_INPUT_DESCR*)currentObject)->analogCommon.Present_Value;
        break;
    case OBJECT_ANALOG_OUTPUT:
        rvalue = (unsigned int)((ANALOG_OUTPUT_DESCR*)currentObject)->analogCommon.Present_Value;
        break;
    case OBJECT_ANALOG_VALUE:
        rvalue = (unsigned int)((ANALOG_VALUE_DESCR*)currentObject)->analogCommon.Present_Value;
        break;
    case OBJECT_BINARY_INPUT:
        rvalue = (unsigned int)((BINARY_INPUT_DESCR*)currentObject)->binaryCommon.Present_Value;
        break;
    case OBJECT_BINARY_OUTPUT:
        rvalue = (unsigned int)((BINARY_OUTPUT_DESCR*)currentObject)->binaryCommon.Present_Value;
        break;
    case OBJECT_BINARY_VALUE:
        rvalue = (unsigned int)((BINARY_VALUE_DESCR*)currentObject)->binaryCommon.Present_Value;
        break;
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
    case OBJECT_MULTISTATE_INPUT:
        rvalue = (unsigned int)((MULTISTATE_INPUT_DESCR*)currentObject)->multistateCommon.Present_Value;
        break;
    case OBJECT_MULTISTATE_OUTPUT:
        rvalue = (unsigned int)((MULTISTATE_OUTPUT_DESCR*)currentObject)->multistateCommon.Present_Value;
        break;
    case OBJECT_MULTISTATE_VALUE:
        rvalue = (unsigned int)((MULTISTATE_VALUE_DESCR*)currentObject)->multistateCommon.Present_Value;
        break;
#endif
    default:
        panic();
        return false;
    }

    *pv = rvalue;
    return true;
}


bool BACapi_Set_PV_double_From_Server_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const double pv)
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);

    if (currentObject == NULL)
    {
        panic();
        return false;
    }

#if 0
    switch (objType)
    {
    case OBJECT_ANALOG_INPUT:
        ((ANALOG_INPUT_DESCR*)currentObject)->analogCommon.Present_Value = (float)pv;
        break;

#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    case OBJECT_ANALOG_OUTPUT:
        ((ANALOG_OUTPUT_DESCR*)currentObject)->analogCommon.Present_Value = (float)pv;
        break;
    case OBJECT_ANALOG_VALUE:
        ((ANALOG_VALUE_DESCR*)currentObject)->analogCommon.Present_Value = (float)pv;
        break;
    case OBJECT_BINARY_INPUT:
        ((BINARY_INPUT_DESCR*)currentObject)->binaryCommon.Present_Value = (BACNET_BINARY_PV)(int)pv;
        break;
    case OBJECT_BINARY_OUTPUT:
        ((BINARY_OUTPUT_DESCR*)currentObject)->binaryCommon.Present_Value = (BACNET_BINARY_PV)(int)pv;
        break;
    case OBJECT_BINARY_VALUE:
        ((BINARY_VALUE_DESCR*)currentObject)->binaryCommon.Present_Value = (BACNET_BINARY_PV)(int)pv;
        break;
    case OBJECT_MULTISTATE_INPUT:
        ((MULTISTATE_INPUT_DESCR*)currentObject)->multistateCommon.Present_Value = (uint16_t)pv;
        break;
    case OBJECT_MULTISTATE_OUTPUT:
        ((MULTISTATE_OUTPUT_DESCR*)currentObject)->multistateCommon.Present_Value = (uint16_t)pv;
        break;
    case OBJECT_MULTISTATE_VALUE:
        ((MULTISTATE_VALUE_DESCR*)currentObject)->multistateCommon.Present_Value = (uint16_t)pv;
        break;
#endif
    default:
        panic();
    }
#endif

    return currentObject->SetPresentValue_real(currentObject, (float)pv);
}


bool BACapi_Set_PV_unsigned_From_Server_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const uint pv)
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);

    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    return currentObject->SetPresentValue_unsigned(currentObject, pv);
}


bool BACapi_Set_PV_boolean_From_Server_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const bool pv)
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);

    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    return currentObject->SetPresentValue_boolean(currentObject, pv);
}


bool BACapi_Set_PV_double_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const float pv)
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);
    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    return currentObject->SetPresentValue_real(currentObject, (float)pv);
}


bool BACapi_Set_PV_unsigned_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const uint pv)
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);
    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    return currentObject->SetPresentValue_unsigned(currentObject, pv);
}


bool BACapi_Set_PV_boolean_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const bool pv)
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);
    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    return currentObject->SetPresentValue_boolean(currentObject, pv);
}


bool BACapi_Set_PV_adv_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const BACNET_APPLICATION_DATA_VALUE *value )
{
    BACNET_OBJECT* currentObject = BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);
    if (currentObject == NULL)
    {
        panic();
        return false;
    }

    return currentObject->SetPresentValue_adv(currentObject, value );
}


void BACapi_Analog_Update_Units(const uint32_t deviceInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance, BACNET_ENGINEERING_UNITS units)
{
    if (!BACapi_isAnalogObject(objType))
    {
        panic();
        return;
    }

    BACNET_OBJECT* currentObject = (BACNET_OBJECT*)BACnet_Find_Object(
        deviceInstance,
        objType,
        objInstance);
    if (currentObject == NULL)
    {
        panic();
        return;
    }

    panic();
    // currentObject->Units = units;
}


BACNET_OBJECT* BACapi_Find_Object(
    const uint devInstance, 
    const BACNET_OBJECT_TYPE objType, 
    const uint objInstance )
{
    return BACnet_Find_Object( devInstance, objType, objInstance );
}


BACNET_OBJECT* BACapi_Establish_Object(
    const uint devInstance, 
    const BACNET_OBJECT_TYPE objType, 
    const uint objInstance, 
    const char* name, 
    const char* description)
{
    BACNET_OBJECT *currentObject = BACapi_Find_Object(devInstance, objType, objInstance);
    if (currentObject) return currentObject;

    return BACapi_Create_Object_with_UserData(devInstance, objType, objInstance, name, description, NULL);
}


BACNET_OBJECT* BACapi_Create_Object(
    const uint devInstance, 
    const BACNET_OBJECT_TYPE objType, 
    const uint objInstance, 
    const char* name, 
    const char* description)
{
    return BACapi_Create_Object_with_UserData(devInstance, objType, objInstance, name, description, NULL);
}


BACNET_OBJECT* BACapi_Create_Object_with_UserData(
    const uint devInstance, 
    const BACNET_OBJECT_TYPE objType, 
    const uint objInstance, 
    const char* name, 
    const char* description, 
    void* userData)
{
    DEVICE_OBJECT_DATA* pDev = Device_Find_Device(devInstance);
    if (pDev == NULL) {
        panicDesc("No device found");
        return NULL;
    }

    BACNET_OBJECT* currentObject;

    switch (objType) {

#if (BACNET_USE_OBJECT_ANALOG_INPUT == 1)
    case OBJECT_ANALOG_INPUT:
        currentObject = Analog_Input_Create(pDev, objInstance, name, UNITS_KILOWATTS);
        break;
#endif

#if (BACNET_USE_OBJECT_ANALOG_OUTPUT == 1)
    case OBJECT_ANALOG_OUTPUT:
        currentObject = Analog_Output_Create(pDev, objInstance, name, UNITS_KILOWATTS);
        break;
#endif

#if (BACNET_USE_OBJECT_ANALOG_VALUE == 1)
    case OBJECT_ANALOG_VALUE:
        currentObject = Analog_Value_Create(pDev, objInstance, name, UNITS_KILOWATTS);
        break;
#endif

#if (BACNET_USE_OBJECT_BINARY_INPUT == 1)
    case OBJECT_BINARY_INPUT:
        currentObject = Binary_Input_Create(pDev, objInstance, name);
        break;
#endif 

#if (BACNET_USE_OBJECT_BINARY_OUTPUT == 1)
    case OBJECT_BINARY_OUTPUT:
        currentObject = Binary_Output_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_BINARY_VALUE == 1)
    case OBJECT_BINARY_VALUE:
        currentObject = Binary_Value_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_CHARACTER_STRING_VALUE == 1)
    case OBJECT_CHARACTERSTRING_VALUE:
        currentObject = CharacterString_Value_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_MULTISTATE_INPUT == 1)
    case OBJECT_MULTISTATE_INPUT:
        currentObject = Multistate_Input_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1)
    case OBJECT_MULTISTATE_OUTPUT:
        currentObject = Multistate_Output_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1)
    case OBJECT_MULTISTATE_VALUE:
        currentObject = Multistate_Value_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
    case OBJECT_NOTIFICATION_CLASS:
        currentObject = Notification_Class_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_CALENDAR == 1)
    case OBJECT_CALENDAR:
        currentObject = Calendar_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_SCHEDULE == 1)
    case OBJECT_SCHEDULE:
        currentObject = Schedule_Create(pDev, objInstance, name);
        break;
#endif

#if (BACNET_USE_OBJECT_TRENDLOG == 1)
    case OBJECT_TRENDLOG:
        panic();
        currentObject = NULL;
        break;
#endif

#if (BACNET_PROTOCOL_REVISION >= 17)
    case OBJECT_NETWORK_PORT:
        currentObject = Network_Port_Create(pDev, objInstance, name );
        break;
#endif

    default:
        panicDesc("Cannot create specified Object Type");
        currentObject = NULL;
        break;
    }

    if (currentObject == NULL) return NULL;

    characterstring_init_ansi(&currentObject->description, description);

    currentObject->userData = userData;

    return currentObject;
}


void BACapi_Delete_Object(BACNET_OBJECT *objToDelete)
{
    LLIST_HDR *llhdr = Find_Descriptor_List(
        objToDelete->pDev2,
        objToDelete->objectType);
    if (llhdr == NULL)
    {
        panic();
        return;
    }

    ll_Remove( llhdr, objToDelete);
    emm_free(objToDelete);
}


BACNET_ENGINEERING_UNITS bits_String_ToUnits(const char* unitsName)
{
    BACNET_ENGINEERING_UNITS found_index;

    // look for generic conversion first, then acronyms
    if (bactext_engineering_unit_enum(unitsName, &found_index))
    {
        return found_index;
    }

    if (bactext_engineering_unit_enum_by_synonym(unitsName, &found_index))
    {
        return found_index;
    }

    dbMessage(DBD_Config,
        DB_ERROR,
        "Could not convert [%s] to BACnet Units",
        unitsName);

    return UNITS_NO_UNITS;
}


void BACnet_Object_Units_Set(BACNET_OBJECT * currentObject, BACNET_ENGINEERING_UNITS units)
{
    switch (currentObject->objectType)
    {
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    case OBJECT_ANALOG_INPUT:
    {
        ANALOG_INPUT_DESCR* anaObj = (ANALOG_INPUT_DESCR*)currentObject;
        anaObj->analogCommon.Units = units;
    }
    break;

    case OBJECT_ANALOG_OUTPUT:
        panic();
        //ANALOG_OUTPUT_DESCR* anaObj = (ANALOG_OUTPUT_DESCR*)currentObject;
        //anaObj->analogCommon.Units = units;
        break;

    case OBJECT_ANALOG_VALUE:
        panic();
        //ANALOG_VALUE_DESCR* anaObj = (ANALOG_VALUE_DESCR*)currentObject;
        //anaObj->analogCommon.Units = units;
        break;
#endif

    default:
        panic();
        break;
    }
}


#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )

void calendar_entry_init(BACNET_CALENDAR_ENTRY * calEntry, CAL_ENTRY_TAG tag)
{
#if ( BAC_DEBUG == 1 )
    calEntry->signature = CAL_ENTRY_SIGNATURE;
#endif 

    calEntry->tag = tag;
}


/*
 *BACnetWeekNDay ::= OCTET STRING (SIZE (3))
    -- first octet month (1..14) where: 1 =January
    -- 13 = odd months
    -- 14 = even months
    -- X'FF' = any month
-- second octet week-of-month(1..9) where: 1 = days numbered 1-7
    -- 2 = days numbered 8-14
    -- 3 = days numbered 15-21
    -- 4 = days numbered 22-28
    -- 5 = days numbered 29-31
    -- 6 = last 7 days of this mon
    -- 7 = any of the 7 days prior
    -- 8 = any of the 7 days prior
    -- 9 = any of the 7 days prior
    -- X'FF' = any week of this m
-- third octet day-of-week (1..7) where: 1 = Monday
    -- 7 = Sunday
    X'FF' = any day of week
 **/

bool datetime_set_weekNday(BACNET_WEEKNDAY * weekNday, uint8_t month, uint8_t weekOfMonth, uint8_t dayOfWeek)
{
    if ((month > 14 || month < 1) && month != 0xff)
    {
        dbMessage(DBD_Application, DB_ERROR, "Illegal Month [%d], needs to be 1-14,255", month);
        return false;
    }
    if ((weekOfMonth > 9 || weekOfMonth < 1) && weekOfMonth != 0xff)
    {
        dbMessage(DBD_Application, DB_ERROR, "Illegal WeekOfMonth [%d], needs to be 1-9,255", weekOfMonth);
        return false;
    }
    if ((dayOfWeek > 7 || dayOfWeek < 1) && dayOfWeek != 0xff)
    {
        dbMessage(DBD_Application, DB_ERROR, "Illegal DayOfWeek [%d], needs to be 1-7,255", dayOfWeek);
        return false;
    }
    weekNday->month = month;
    weekNday->dayofweek = dayOfWeek;
    weekNday->weekofmonth = weekOfMonth;
    return true;
}


void calendar_entry_init_date(BACNET_CALENDAR_ENTRY * calEntry, BACNET_DATE * date)
{
    calendar_entry_init(calEntry, CALENDAR_ENTRY_DATE);
    datetime_copy_date(&calEntry->CalEntryChoice.date, date);
}


void calendar_entry_init_range(BACNET_CALENDAR_ENTRY * calEntry, BACNET_DATE_RANGE * dateRange)
{
    calendar_entry_init(calEntry, CALENDAR_ENTRY_RANGE);
    datetime_copy_date(&calEntry->CalEntryChoice.range.startdate, &dateRange->startdate);
    datetime_copy_date(&calEntry->CalEntryChoice.range.enddate, &dateRange->enddate);
}


void calendar_entry_init_weekNday(BACNET_CALENDAR_ENTRY * calEntry, BACNET_WEEKNDAY * weekNday)
{
    calendar_entry_init(calEntry, CALENDAR_ENTRY_WEEKNDAY);
    memcpy(&calEntry->CalEntryChoice.weekNday, weekNday, sizeof(BACNET_WEEKNDAY));
}


void calendar_entry_copy(BACNET_CALENDAR_ENTRY * entry1, BACNET_CALENDAR_ENTRY * entry2)
{
#if ( BAC_DEBUG == 1 )
    if (entry2->signature != CAL_ENTRY_SIGNATURE) {
        panic();
        return;
    }
#endif 

    memcpy(entry1, entry2, sizeof(BACNET_CALENDAR_ENTRY));
}


bool date_compare_weekndays(
    BACNET_WEEKNDAY * weekNday1,
    BACNET_WEEKNDAY * weekNday2)
{
    return (memcmp(weekNday1, weekNday2, sizeof(BACNET_WEEKNDAY)) == 0);
}


static bool compare_calendar_entrys(BACNET_CALENDAR_ENTRY * entry1, BACNET_CALENDAR_ENTRY * entry2)
{
    bool  active = false;

    if (entry1->tag != entry2->tag) return false;

    switch (entry1->tag) {
    case CALENDAR_ENTRY_DATE:
        return (datetime_compare_date(&entry1->CalEntryChoice.date, &entry2->CalEntryChoice.date) == 0);

    case CALENDAR_ENTRY_RANGE:
        if (datetime_compare_date(&entry1->CalEntryChoice.range.startdate, &entry2->CalEntryChoice.range.startdate) != 0) return false;
        return (datetime_compare_date(&entry1->CalEntryChoice.range.enddate, &entry2->CalEntryChoice.range.enddate) == 0);

    case CALENDAR_ENTRY_WEEKNDAY:
        return date_compare_weekndays(&entry1->CalEntryChoice.weekNday, &entry2->CalEntryChoice.weekNday);
    }
    panic();
    return false;
}


bool BACapi_Calendar_Datelist_Add(CALENDAR_DESCR * calObj, BACNET_CALENDAR_ENTRY * bacCalEntry)
{
    int firstFree = -1;

#if (BAC_DEBUG == 1 )
    if (calObj->common.objectType != OBJECT_CALENDAR) {
        panic();
        return false;
    }
    if (bacCalEntry->signature != CAL_ENTRY_SIGNATURE) {
        panic();
        return false;
    }
#endif 

    // confirm no duplicates...
    for (int index = 0; index < MAX_CALENDAR_EVENTS; index++) {
        if (calObj->dateList[index].tag == CALENDAR_ENTRY_NONE)
        {
            if (firstFree < 0) firstFree = index;
        }
        else
        {
            if (compare_calendar_entrys(&calObj->dateList[index], bacCalEntry))
            {
                // we have a match, do not attempt to add a duplicate
                return true;
            }
        }
    }

    if (firstFree < 0)
    {
        // means that we did not notice a free entry when we searched for duplicates above. i.e. we are full!
        dbMessage(DBD_ALL,
            DB_ERROR,
            "Ran out of space in calendar list in [%s]",
            characterstring_value(&calObj->common.objectName));
        return false;
    }

    calendar_entry_copy(&calObj->dateList[firstFree], bacCalEntry);

    return true;
}


void BACapi_Special_Event_Period_Calendar_Reference_Add(
    BACNET_SPECIAL_EVENT * specialEvent,
    uint32_t objectInstance)
{
    specialEvent->period.type = EXCEPTION_CALENDAR_REFERENCE;
    specialEvent->period.calendarReferenceInstance = objectInstance;
}


void BACapi_Special_Event_Period_Calendar_Entry_Add(
    BACNET_SPECIAL_EVENT * specialEvent,
    BACNET_CALENDAR_ENTRY * calEntry)
{
    specialEvent->period.type = EXCEPTION_CALENDAR_ENTRY;
    memcpy(&specialEvent->period.calendarEntry, calEntry, sizeof(BACNET_CALENDAR_ENTRY));
}


void BACapi_Value_Set_Real(
    BACNET_APPLICATION_DATA_VALUE * timeValue,
    double value)
{}

void BACapi_Time_Set(
    BACNET_TIME * timeValue,
    uint hours, uint mins, uint secs)
{}



//void BACapi_Special_Event_Calendar_Entry_Add(
//    BACNET_SPECIAL_EVENT * specialEvent,
//    BACNET_CALENDAR_ENTRY * timeValue)
//{
//    panic();
//}

void BACapi_Special_Event_TimeValue_Add(
    BACNET_SPECIAL_EVENT * specialEvent,
    BACNET_TIME_VALUE * timeValue)
{
    if (!emm_check_alloc_two(timeValue, specialEvent))
    {
        panic();
        return;
    }

    if (specialEvent->ux_TimeValues1 >= MX_SPECIAL_EVENT_TIME_VALUES)
    {
        panic();
        return;
    }

    memcpy(&specialEvent->listOfTimeValues[specialEvent->ux_TimeValues1++], timeValue, sizeof(BACNET_TIME_VALUE));
    // need to add to a list
    emm_free(timeValue);
}

void BACapi_Schedule_Special_Event_Add(
    SCHEDULE_DESCR * schedObj,
    BACNET_SPECIAL_EVENT * specialEvent)
{
    if (!emm_check_alloc_two(schedObj, specialEvent))
    {
        panic();
        return;
    }

    if (schedObj->ux_special_events < MX_EXCEPTION_SCHEDULE)
    {
        memcpy(&schedObj->Exception_Schedule[schedObj->ux_special_events++], specialEvent, sizeof(BACNET_SPECIAL_EVENT));
        emm_free(specialEvent);
        return;
    }

    // too many items
    emm_free(specialEvent);
    panic();
}


BACNET_APPLICATION_DATA_VALUE* BACapi_adv_Real(double rval)
{
    BACNET_APPLICATION_DATA_VALUE* tadv = (BACNET_APPLICATION_DATA_VALUE*)emm_calloc(sizeof(BACNET_APPLICATION_DATA_VALUE));
    if (tadv == NULL) return NULL;

    tadv->tag = BACNET_APPLICATION_TAG_REAL;
    tadv->type.Real = (float)rval;
    return tadv;
}
#endif

double BACapi_Get_adv_double(const BACNET_APPLICATION_DATA_VALUE * adv)
{
    switch (adv->tag)
    {
    case BACNET_APPLICATION_TAG_NULL: 
        return 0.;
        break;
    case BACNET_APPLICATION_TAG_BOOLEAN: 
        return (double)adv->type.Boolean;
        break;
    case BACNET_APPLICATION_TAG_UNSIGNED_INT:
        return (double)adv->type.Unsigned_Int;
        break;
    case BACNET_APPLICATION_TAG_SIGNED_INT: 
        return (double)adv->type.Signed_Int;
        break;
    case BACNET_APPLICATION_TAG_REAL: 
        return (double)adv->type.Real;
        break;
    case BACNET_APPLICATION_TAG_DOUBLE: 
        return (double)adv->type.Double;
        break;
    case BACNET_APPLICATION_TAG_ENUMERATED: 
        return (double)adv->type.Enumerated;
        break;
    case BACNET_APPLICATION_TAG_CHARACTER_STRING: 
        // figure out a valid use case before enabling this.
        panic();
        // return atof(adv->type.Character_String.value);
        break;
        // the following make no sense
    case BACNET_APPLICATION_TAG_OCTET_STRING:
    case BACNET_APPLICATION_TAG_BIT_STRING:
        panic();
        break;
    default:
        panic();
        break;
    }
    return 0.;
}


BACNET_UNSIGNED_INTEGER BACapi_Get_adv_unsigned(const BACNET_APPLICATION_DATA_VALUE* adv)
{
    switch (adv->tag)
    {
    case BACNET_APPLICATION_TAG_NULL:
        return 0;
        break;
    case BACNET_APPLICATION_TAG_BOOLEAN:
        return (uint)adv->type.Boolean;
        break;
    case BACNET_APPLICATION_TAG_UNSIGNED_INT:
        return adv->type.Unsigned_Int;
        break;
    case BACNET_APPLICATION_TAG_SIGNED_INT:
        return (uint)adv->type.Signed_Int;
        break;
    case BACNET_APPLICATION_TAG_REAL:
        return (uint)adv->type.Real;
        break;
    case BACNET_APPLICATION_TAG_DOUBLE:
        return (uint)adv->type.Double;
        break;
    case BACNET_APPLICATION_TAG_ENUMERATED:
        return (uint)adv->type.Enumerated;
        break;
    case BACNET_APPLICATION_TAG_CHARACTER_STRING:
        // figure out a valid use case before enabling this.
        panic();
        // return atof(adv->type.Character_String.value);
        break;
        // the following make no sense
    case BACNET_APPLICATION_TAG_OCTET_STRING:
    case BACNET_APPLICATION_TAG_BIT_STRING:
        panic();
        break;
    default:
        panic();
        break;
    }
    return 0 ;
}


#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )

BACNET_TIME* BACapi_adv_Time(
    uint hrs,
    uint mins,
    uint secs)
{
    BACNET_TIME* tt = (BACNET_TIME*)emm_calloc(sizeof(BACNET_TIME));
    if (tt == NULL) return NULL;
    tt->hour = hrs;
    tt->min = mins;
    tt->sec = secs;
    tt->hundredths = 0;
    return tt;
}


BACNET_TIME_VALUE* BACapi_adv_TimeValue(
    BACNET_TIME * time,
    BACNET_APPLICATION_DATA_VALUE * adv)
{
    if (!emm_check_alloc_two(time, adv))
    {
        panic();
        return NULL;
    }
    BACNET_TIME_VALUE* tv = (BACNET_TIME_VALUE*)emm_calloc(sizeof(BACNET_TIME_VALUE));
    if (tv == NULL)
    {
        emm_free_two(time, adv);
        return NULL;
    }
    memcpy(&tv->Time, time, sizeof(BACNET_TIME));
    memcpy(&tv->Value, adv, sizeof(BACNET_APPLICATION_DATA_VALUE));
    emm_free_two(time, adv);
    return tv;
}


int bactime_isequal(
    BACNET_TIME * time1,
    BACNET_TIME * time2)
{
    if (time1->hour != time2->hour) return false;
    if (time1->min != time2->min) return false;
    if (time1->sec != time2->sec) return false;
    return true;
}


SCHEDULE_DESCR* BACapi_Iterate_Schedule(const uint32_t deviceInstance, const SCHEDULE_DESCR *priorSchedule)
{
    // todo 0 - static review
    static DEVICE_OBJECT_DATA *storedDevice;
    static uint32_t storedInstance = 0xffffffff;

    // a bit of caching to improve performance
    if (deviceInstance != storedInstance)
    {
        storedDevice = Device_Find_Device(deviceInstance);
        storedInstance = deviceInstance;
    }
    if (!storedDevice) return NULL;

    if (priorSchedule) return (SCHEDULE_DESCR *)ll_Next(&storedDevice->Schedule_Descriptor_List, priorSchedule);

    return (SCHEDULE_DESCR *)ll_First(&storedDevice->Schedule_Descriptor_List);
}





int BACapi_Schedule_Daily_Schedule_Find(
    SCHEDULE_DESCR * schedObj,
    BACNET_DAYS_OF_WEEK  weekDay,
    BACNET_TIME * time,
    bool used)
{
    if (weekDay >= MAX_BACNET_DAYS_OF_WEEK)
    {
        panic();
        return -1;
    }
    for (int i = 0; i < BACNET_DAILY_SCHEDULE_SIZE; i++)
    {
        if (used)
        {
            if (schedObj->Weekly_Schedule[weekDay].used[i] == true)
            {
                if (bactime_isequal(&schedObj->Weekly_Schedule[weekDay].Time_Values[i].Time, time))
                {
                    return i;
                }
            }
        }
        else
        {
            if (schedObj->Weekly_Schedule[weekDay].used[i] == false)
            {
                return i;
            }
        }
    }
    return -1;
}



void BACapi_Schedule_Weekly_Schedule_Remove(
    SCHEDULE_DESCR * schedObj,
    BACNET_DAYS_OF_WEEK  weekDay,
    BACNET_TIME * time)
{
    int index = BACapi_Schedule_Daily_Schedule_Find(schedObj, weekDay, time, true);
    if (index > 0)
    {
        schedObj->Weekly_Schedule[weekDay].used[index] = false;
    }
    else
    {
        // trying to remove unknown schedule item;
        panic();
    }
}


void BACapi_Time_Value_Set(BACNET_TIME_VALUE * timeValue,
    BACNET_TIME * time,
    BACNET_APPLICATION_DATA_VALUE * applicationDataValue)
{
    if (!emm_check_alloc_two(time, applicationDataValue))
    {
        panic();
        return;
    }
    timeValue->Time.hour = time->hour;
    timeValue->Time.min = time->min;
    timeValue->Time.sec = time->sec;
    bacapp_copy(&timeValue->Value, applicationDataValue);

    emm_free(time);
    emm_free(applicationDataValue);
}


void BACapi_Daily_Schedule_Add(
    BACNET_DAILY_SCHEDULE * dailySchedule,
    BACNET_TIME_VALUE * timeValue)
{
    if (!emm_check_alloc_two(dailySchedule, timeValue))
    {
        panic();
        return;
    }
    // int index = BACapi_Schedule_Weekly_Schedule_Find
}


void BACapi_Schedule_Weekly_Schedule_Add(
    SCHEDULE_DESCR * schedObj,
    BACNET_DAYS_OF_WEEK  weekDay,
    BACNET_TIME_VALUE * advTimeValue)                  // this value must be created and assigned to using one of the BACapi_adv_...() functions, and never reused, freed etc.
{
    if (!emm_check_alloc_two(schedObj, advTimeValue))
    {
        panic();
        return;
    }

    // BACNET_DAILY_SCHEDULE   dailySchedule;
    // BACNET_TIME_VALUE       timeValue2;

    int index = BACapi_Schedule_Daily_Schedule_Find(
        schedObj,
        weekDay,
        &advTimeValue->Time,
        false);
    if (index >= 0)
    {
        // BACapi_Daily_Schedule_Add(&dailySchedule, advTimeValue );
        // emm_free(advTimeValue);

        memcpy(&schedObj->Weekly_Schedule[weekDay].Time_Values[index], advTimeValue, sizeof(BACNET_TIME_VALUE));
        schedObj->Weekly_Schedule[weekDay].used[index] = true;
    }
    else
    {
        // no more room
        panic();
    }
}


BACNET_SPECIAL_EVENT_PERIOD* BACapi_adv_Special_Event_Period_Calendar_Reference(
    uint32_t calendarObjectInstance)
{
    BACNET_SPECIAL_EVENT_PERIOD* tv = (BACNET_SPECIAL_EVENT_PERIOD*)emm_calloc(sizeof(BACNET_SPECIAL_EVENT_PERIOD));
    if (tv == NULL) return NULL;
    tv->type = EXCEPTION_CALENDAR_REFERENCE;
    tv->calendarReferenceInstance = calendarObjectInstance;
    return tv;
}

BACNET_SPECIAL_EVENT_PERIOD* BACapi_adv_Special_Event_Period_Calendar_Entry(
    BACNET_CALENDAR_ENTRY * calendarEntry)
{
    if (!emm_check_alloc(calendarEntry))
    {
        panic();
        return NULL;
    }
    BACNET_SPECIAL_EVENT_PERIOD* tv = (BACNET_SPECIAL_EVENT_PERIOD*)emm_calloc(sizeof(BACNET_SPECIAL_EVENT_PERIOD));
    if (tv == NULL)
    {
        emm_free(calendarEntry);
        return NULL;
    }
    tv->type = EXCEPTION_CALENDAR_ENTRY;
    memcpy(&tv->calendarEntry, calendarEntry, sizeof(BACNET_CALENDAR_ENTRY));
    emm_free(calendarEntry);
    return tv;
}

BACNET_SPECIAL_EVENT* BACapi_adv_Special_Event(
    BACNET_SPECIAL_EVENT_PERIOD * specialEventPeriod,
    uint priority)
{
    if (!emm_check_alloc(specialEventPeriod))
    {
        panic();
        return NULL;
    }
    if (specialEventPeriod == NULL) {
        panic();
        return NULL;
    }
    if (!(priority >= 1 && priority <= 16))
    {
        dbMessage(DBD_Application, DB_ERROR, "Priority [%d] not between 1 and 16", priority);
        emm_free(specialEventPeriod);
        return NULL;
    }
    BACNET_SPECIAL_EVENT* tv = (BACNET_SPECIAL_EVENT*)emm_calloc(sizeof(BACNET_SPECIAL_EVENT));
    if (tv == NULL)
    {
        emm_free(specialEventPeriod);
        return NULL;
    }
    memcpy(&tv->period, specialEventPeriod, sizeof(BACNET_SPECIAL_EVENT_PERIOD));
    emm_free(specialEventPeriod);
    // todo 3 - range check?
    tv->priority = (uint8_t)priority;
    return tv;
}

BACNET_CALENDAR_ENTRY* BACapi_adv_CalendarEntry_WeekNDay(
    uint month,
    uint weekOfMonth,
    uint dayOfWeek)
{
    BACNET_CALENDAR_ENTRY* tv = (BACNET_CALENDAR_ENTRY*)emm_calloc(sizeof(BACNET_CALENDAR_ENTRY));
    if (tv == NULL) return NULL;
    tv->tag = CALENDAR_ENTRY_WEEKNDAY;
    // todo 4 - add sanity checks here
    tv->CalEntryChoice.weekNday.month = month;
    tv->CalEntryChoice.weekNday.weekofmonth = weekOfMonth;
    tv->CalEntryChoice.weekNday.dayofweek = dayOfWeek;
    return tv;
}

BACNET_CALENDAR_ENTRY* BACapi_adv_CalendarEntry_Date(
    uint year,
    uint month,
    uint day,
    BACNET_WEEKDAY wday)
{
    BACNET_CALENDAR_ENTRY* tv = (BACNET_CALENDAR_ENTRY*)emm_calloc(sizeof(BACNET_CALENDAR_ENTRY));
    if (tv == NULL) return NULL;
    tv->tag = CALENDAR_ENTRY_DATE;
    // todo 4 - add sanity checks here
    tv->CalEntryChoice.date.month = month;
    tv->CalEntryChoice.date.day = day;
    tv->CalEntryChoice.date.year = year;
    tv->CalEntryChoice.date.wday = wday;
    return tv;
}

BACNET_CALENDAR_ENTRY* BACapi_adv_CalendarEntry_DateRange(
    uint startYear,
    uint startMonth,
    uint startDay,
    uint endYear,
    uint endMonth,
    uint endDay)
{
    BACNET_CALENDAR_ENTRY* tv = (BACNET_CALENDAR_ENTRY*)emm_calloc(sizeof(BACNET_CALENDAR_ENTRY));
    if (tv == NULL) return NULL;
    tv->tag = CALENDAR_ENTRY_RANGE;
    // todo 4 - add sanity checks here
    tv->CalEntryChoice.range.startdate.month = startMonth;
    tv->CalEntryChoice.range.startdate.day = startDay;
    tv->CalEntryChoice.range.startdate.year = startYear;
    tv->CalEntryChoice.range.enddate.month = endMonth;
    tv->CalEntryChoice.range.enddate.day = endDay;
    tv->CalEntryChoice.range.enddate.year = endYear;
    return tv;
}
#endif // #if ( BACNET_USE_OBJECT_SCHEDULE == 1 )



#if ( BAC_DEBUG == 1 )

void Test_BACapi(void)
{
    uint32_t    testDeviceInstance = 100;
    uint32_t    testDeviceInstance2 = 800;

    panic();
    // InitBACnetDataStructures();

    PORT_SUPPORT *datalink = datalink_initCommon("Virtual", BPT_VIRT );
    if (!datalink) panic();

    DEVICE_OBJECT_DATA* pDev = Device_Create_Device_Server(datalink, testDeviceInstance, "BACnetdevName", "Description devDesc", "Model Name", NULL );
    if (!pDev) panic();

#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
    SCHEDULE_DESCR *sched = (SCHEDULE_DESCR * ) BACapi_Establish_Object(testDeviceInstance, OBJECT_SCHEDULE, 1000, "Sched", "Nodesc");
    if (!sched) panic();

    BACNET_OBJECT *sched2 = BACapi_Find_Object(testDeviceInstance, OBJECT_SCHEDULE, 1000 );
    if (!sched2) panic();

    assert(pDev->Schedule_Descriptor_List.count == 1);

    BACapi_Delete_Object(sched2);

    assert(pDev->Schedule_Descriptor_List.count == 0);


    // test iterator

    pDev = Device_Create_Device_Server(datalink, testDeviceInstance2, "BACnetdevName", "Description devDesc", "Model Name", NULL);
    if (!pDev) panic();

    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance2, OBJECT_SCHEDULE, 1001, "Sched1", "Nodesc");
    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance2, OBJECT_SCHEDULE, 1002, "Sched2", "Nodesc");

    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance, OBJECT_SCHEDULE, 1000, "Sched0", "Nodesc");
    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance, OBJECT_SCHEDULE, 1001, "Sched1", "Nodesc");
    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance, OBJECT_SCHEDULE, 1002, "Sched2", "Nodesc");
    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance, OBJECT_SCHEDULE, 1003, "Sched3", "Nodesc");
    sched = (SCHEDULE_DESCR *)BACapi_Establish_Object(testDeviceInstance, OBJECT_SCHEDULE, 1004, "Sched4", "Nodesc");

    pDev = Device_Find_Device(testDeviceInstance2);
    assert(pDev->Schedule_Descriptor_List.count == 2);
    for (sched = BACapi_Iterate_Schedule(testDeviceInstance2, NULL);
        sched;
        sched = BACapi_Iterate_Schedule(testDeviceInstance2, sched))
    {
        printf("Sched = Dev:%07d  Sched:%s\n", pDev->bacObj.objectInstance, sched->common.objectName.value);
    }

    // delete those objects
    while (sched = BACapi_Iterate_Schedule(testDeviceInstance2, NULL))
    {
        BACapi_Delete_Object((BACNET_OBJECT *)sched);
    }
    assert(pDev->Schedule_Descriptor_List.count == 0);

    printf("---------------------\n");

    pDev = Device_Find_Device(testDeviceInstance );
    assert(pDev->Schedule_Descriptor_List.count == 5);
    for (sched = BACapi_Iterate_Schedule(testDeviceInstance, NULL);
        sched;
        sched = BACapi_Iterate_Schedule(testDeviceInstance, sched))
    {
        printf("Sched = Dev:%07d  Sched:%s\n", pDev->bacObj.objectInstance, sched->common.objectName.value);
    }

    // delete those objects
    while (sched = BACapi_Iterate_Schedule(testDeviceInstance2, NULL))
    {
        BACapi_Delete_Object((BACNET_OBJECT *)sched);
    }
#endif // ( BACNET_USE_OBJECT_SCHEDULE == 1 )

    pDev = Device_Find_Device(testDeviceInstance2);
    assert(pDev->Schedule_Descriptor_List.count == 0);

    Device_Delete_Device(pDev, "bacnet" );

    printf("---------------------\n");


    pDev = Device_Find_Device(testDeviceInstance);
    Device_Delete_Device(pDev, "bacnet" );

}
#endif // BAC_DEBUG
