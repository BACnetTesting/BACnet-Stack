/**************************************************************************
 *
 * Copyright (C) 2005,2006,2009 Steve Karg <skarg@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************************
 *
 *   Modifications Copyright (C) 2016 ConnectEx, Inc
 *
 *   March 3, 2016    EKH    AddListElement / RemoveListElement
 *                           This file has been modified to support the AddListElement and RemoveListElement
 *                           services and the supporting code for these services by ConnectEx, Inc.
 *                           Questions regarding this can be directed to: info@connect-ex.com
 *
 *****************************************************************************************
 *
 *   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
 *
 *   July 1, 2017    BITS    Modifications to this file have been made in compliance
 *                           with original licensing.
 *
 *   This file contains changes made by BACnet Interoperability Testing
 *   Services, Inc. These changes are subject to the permissions,
 *   warranty terms and limitations above.
 *   For more information: info@bac-test.com
 *   For access to source code:  info@bac-test.com
 *          or      www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

/*

2016.03.22	EKH		AddListElement / RemoveListElement
	This file has been modified to support the AddListElement and RemoveListElement
	services and the supporting code for these services by ConnectEx, Inc.
	Questions regarding this can be directed to: info@connect-ex.com

 */

/** @file device.c Base "class" for handling all BACnet objects belonging
 *                 to a BACnet device, as well as Device-specific properties. */

#include <stdbool.h>

#include <stdint.h>
#include <string.h>     /* for memmove */
#include <time.h>       /* for timezone, localtime */
#include "configProj.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacenum.h"
#include "bacnet/bacapp.h"
#include "configProj.h"         /* the custom stuff */
#include "bacnet/apdu.h"
#include "bacnet/wp.h"             /* WriteProperty handling */
#include "bacnet/rp.h"             /* ReadProperty handling */
#include "bacnet/dcc.h"            /* DeviceCommunicationControl handling  */ 
#include "bacnet/version.h"
#include "bacnet/basic/object/device.h"         /* me  */ 
#include "bacnet/basic/services.h"
#include "bacnet/bits/util/multipleDatalink.h"
#include "bacnet/basic/binding/address.h"
#include "eLib/util/emm.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"
#include "bacnet/basic/service/h_wp.h"
#ifdef AUTOCREATE_SITE
#include "appApi.h"
#endif

#if  ( BACDL_MSTP == 1)
#include "bacnet/datalink/dlmstp.h"
#endif //  ( BACDL_MSTP == 1)

#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
#include "ai.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
#include "ao.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
#include "av.h"
#endif
#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
#include "bi.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )
#include "bo.h"
#endif
#if ( BACNET_USE_OBJECT_CALENDAR == 1 )
#include "calendar.h"
#endif
#if ( BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1 )
#include "alert_enrollment.h"
#endif
#if ( BACNET_USE_OBJECT_BINARY_VALUE == 1 )
#include "bv.h"
#endif

 //#if defined ( BACAPP_LIGHTING_COMMAND )
 //#include "channel.h"
 //#include "command.h"
 //#endif
 //#include "csv.h"
#if ( BACNET_USE_OBJECT_INTEGER_VALUE == 1 )
#include "iv.h"
#endif
//#include "lc.h"
//#include "lsp.h"
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
//#include "osv.h"
#if ( BACNET_USE_OBJECT_POSITIVE_INTEGER_VALUE == 1 )
#include "piv.h"
#endif
#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
#include "schedule.h"
#endif
#if ( BACNET_USE_OBJECT_TRENDLOG == 1 )
#include "trendlog.h"
#endif
#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#endif /* (INTRINSIC_REPORTING_B == 1) */
#if (BACFILE == 1)
#include "bacfile.h"
#endif /* defined(BACFILE) */
#include "bacnet/basic/object/netport.h"

#include "bacnet/bacaddr.h"
#include "bacnet/iam.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"
#include "eLib/util/eLibDebug.h"
#include "osLayer.h"
#include "bacnet/bactext.h"
#include "bacnet/basic/tsm/tsm.h"

//#if defined(_WIN32)
///* Not included in time.h as specified by The Open Group */
///* Difference from UTC and local standard time */
//long int timezone;
//#endif

// bits_mutex_extern(stackLock);

// extern DEVICE_OBJECT_DATA applicationDevice;

/* local forward (semi-private) and external prototypes */
//int Device_Read_Property_Local(
//    DEVICE_OBJECT_DATA *pDev,
//    BACNET_READ_PROPERTY_DATA * rpdata);
//
//bool Device_Write_Property_Local(
//    DEVICE_OBJECT_DATA *pDev,
//    BACNET_WRITE_PROPERTY_DATA * wp_data);

//extern int Routed_Device_Read_Property_Local(
//    DEVICE_OBJECT_DATA *pDev,
//    BACNET_READ_PROPERTY_DATA * rpdata);
//
//extern bool Routed_Device_Write_Property_Local(
//    DEVICE_OBJECT_DATA *pDev,
//    BACNET_WRITE_PROPERTY_DATA * wp_data);

const char* BACnet_Version = BACNET_VERSION_TEXT ;
extern volatile struct mstp_port_struct_t MSTP_Port;

// all of Steve Karg's global statics have to be put into structures since we will want to support
// multiple devices when using Virtual Devices with routing.
// However, there will will always be one Application Entity, representing either the router device itself, when routing,
// or the BACnet Server device when not routing. Even BACnet Clients need an application entity in case someone
// detects and polls the client when the client is online.
DEVICE_OBJECT_DATA *applicationEntity;

// server devices = virtual, real, application entity devices, attached to an appropriate datalink during configuration
// client devices = client side devices that contain parameters for polling, only bound to a datalink at runtime
LLIST_HDR serverDeviceCB;

/* may be overridden by outside table */
// static object_functions_t *Object_Table;

static object_functions_t My_Object_Table[] =
{
    {
    OBJECT_DEVICE,
    NULL /* Init - don't init Device or it will recurse! */ ,
    Device_Count,
    Device_Index_To_Instance,
    Device_Valid_Object_Instance_Number,
    Device_Object_Name,
    Device_Read_Property_Local,
    Device_Write_Property_Local,
    Device_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    DeviceGetRRInfo,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    Device_Add_List_Element_Local,
    Device_Remove_List_Element_Local,
#endif

#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },

#if (BACNET_PROTOCOL_REVISION >= 17)
    {
    OBJECT_NETWORK_PORT,
    Network_Port_Init,
    Network_Port_Count,
    Network_Port_Index_To_Instance,
    Network_Port_Valid_Instance,
    Network_Port_Object_Name,
    Network_Port_Read_Property,
    Network_Port_Write_Property,
    Network_Port_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if (INTRINSIC_REPORTING_B == 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1 )
    {
    OBJECT_ALERT_ENROLLMENT,
    Alert_Enrollment_Init,
    Alert_Enrollment_Count,
    Alert_Enrollment_Index_To_Instance,
    Alert_Enrollment_Valid_Instance,
    Alert_Enrollment_Object_Name,
    Alert_Enrollment_Read_Property,
    Alert_Enrollment_Write_Property,
    Alert_Enrollment_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif

#if  (LIST_MANIPULATION == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif

#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */,
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
    {
    OBJECT_ANALOG_INPUT,
    Analog_Input_Init,
    Analog_Input_Count,
    Analog_Input_Index_To_Instance,
    Analog_Input_Valid_Instance,
    Analog_Input_Object_Name,
    Analog_Input_Read_Property,
    Analog_Input_Write_Property,
    Analog_Input_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    Analog_Input_Encode_Value_List,
    Analog_Input_Change_Of_Value,
    Analog_Input_Change_Of_Value_Clear,
#endif
#if  (BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B == 1)
    Analog_Input_Intrinsic_Reporting
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    {
    OBJECT_ANALOG_OUTPUT,
    Analog_Output_Init,
    Analog_Output_Count,
    Analog_Output_Index_To_Instance,
    Analog_Output_Valid_Instance,
    Analog_Output_Object_Name,
    Analog_Output_Read_Property,
    Analog_Output_Write_Property,
    Analog_Output_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL, /* ReadRangeInfo */
#endif
    NULL, /* Iterator */

#if ( BACNET_SVC_COV_B == 1)
    Analog_Output_Encode_Value_List,
    Analog_Output_Change_Of_Value,
    Analog_Output_Change_Of_Value_Clear,
#endif

#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    Analog_Output_Intrinsic_Reporting
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_ANALOG_VALUE == 1 )
    {
    OBJECT_ANALOG_VALUE,
    Analog_Value_Init,
    Analog_Value_Count,
    Analog_Value_Index_To_Instance,
    Analog_Value_Valid_Instance,
    Analog_Value_Object_Name,
    Analog_Value_Read_Property,
    Analog_Value_Write_Property,
    Analog_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    Analog_Value_Encode_Value_List,
    Analog_Value_Change_Of_Value,
    Analog_Value_Change_Of_Value_Clear,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    Analog_Value_Intrinsic_Reporting
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_BINARY_INPUT == 1 )
    {
    OBJECT_BINARY_INPUT,
    Binary_Input_Init,
    Binary_Input_Count,
    Binary_Input_Index_To_Instance,
    Binary_Input_Valid_Instance,
    Binary_Input_Object_Name,
    Binary_Input_Read_Property,
    Binary_Input_Write_Property,
    Binary_Input_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    Binary_Input_Encode_Value_List,
    Binary_Input_Change_Of_Value,
    Binary_Input_Change_Of_Value_Clear,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )
    {
    OBJECT_BINARY_OUTPUT,
    Binary_Output_Init,
    Binary_Output_Count,
    Binary_Output_Index_To_Instance,
    Binary_Output_Valid_Instance,
    Binary_Output_Object_Name,
    Binary_Output_Read_Property,
    Binary_Output_Write_Property,
    Binary_Output_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    Binary_Output_Encode_Value_List,
    Binary_Output_Change_Of_Value,
    Binary_Output_Change_Of_Value_Clear,
#endif

#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if ( INTRINSIC_REPORTING_B == 1 )
    NULL /* Intrinsic Reporting */
#endif
    },
#endif // BACNET_USE_OBJECT_BINARY_OUTPUT

#if ( BACNET_USE_OBJECT_BINARY_VALUE == 1 )
    {
    OBJECT_BINARY_VALUE,
    Binary_Value_Init,
    Binary_Value_Count,
    Binary_Value_Index_To_Instance,
    Binary_Value_Valid_Instance,
    Binary_Value_Object_Name,
    Binary_Value_Read_Property,
    Binary_Value_Write_Property,
    Binary_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1)
    Binary_Value_Encode_Value_List,
    Binary_Value_Change_Of_Value,
    Binary_Value_Change_Of_Value_Clear,
#endif

#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_CHARACTERSTRING_VALUE == 1 )
    {
    OBJECT_CHARACTERSTRING_VALUE,
    CharacterString_Value_Init,
    CharacterString_Value_Count,
    CharacterString_Value_Index_To_Instance,
    CharacterString_Value_Valid_Instance,
    CharacterString_Value_Object_Name,
    CharacterString_Value_Read_Property,
    CharacterString_Value_Write_Property,
    CharacterString_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( 0 )
    {
    OBJECT_COMMAND,
    Command_Init,
    Command_Count,
    Command_Index_To_Instance,
    Command_Valid_Instance,
    Command_Object_Name,
    Command_Read_Property,
    Command_Write_Property,
    Command_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_List */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_INTEGER_VALUE == 1 )
    {
    OBJECT_INTEGER_VALUE,
    Integer_Value_Init,
    Integer_Value_Count,
    Integer_Value_Index_To_Instance,
    Integer_Value_Valid_Instance,
    Integer_Value_Object_Name,
    Integer_Value_Read_Property,
    Integer_Value_Write_Property,
    Integer_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    Integer_Value_Encode_Value_List,
    Integer_Value_Change_Of_Value,
    Integer_Value_Change_Of_Value_Clear,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if 0
    { OBJECT_NETWORK_PORT,
    Network_Port_Init,
    Network_Port_Count,
    Network_Port_Index_To_Instance,
    Network_Port_Valid_Instance,
    Network_Port_Object_Name,
    Network_Port_Read_Property,
    Network_Port_Write_Property,
    Network_Port_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
        NULL /* Value_List */ ,
        NULL /* COV */ ,
        NULL /* COV Clear */ ,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if (INTRINSIC_REPORTING_B == 1) || ( BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1 )
    {
    OBJECT_NOTIFICATION_CLASS,
    Notification_Class_Init,
    Notification_Class_Count,
    Notification_Class_Index_To_Instance,
    Notification_Class_Valid_Instance,
    Notification_Class_Object_Name,
    Notification_Class_Read_Property,
    Notification_Class_Write_Property,
    Notification_Class_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_List */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif

#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    Notification_Class_Add_List_Element,
    Notification_Class_Remove_List_Element,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_LIFE_SAFETY == 1 )
    {
    OBJECT_LIFE_SAFETY_POINT,
    Life_Safety_Point_Init,
    Life_Safety_Point_Count,
    Life_Safety_Point_Index_To_Instance,
    Life_Safety_Point_Valid_Instance,
    Life_Safety_Point_Object_Name,
    Life_Safety_Point_Read_Property,
    Life_Safety_Point_Write_Property,
    Life_Safety_Point_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_List */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if  (LIST_MANIPULATION == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if (BACNET_USE_OBJECT_LOAD_CONTROL == 1)
    {
    OBJECT_LOAD_CONTROL,
    Load_Control_Init,
    Load_Control_Count,
    Load_Control_Index_To_Instance,
    Load_Control_Valid_Instance,
    Load_Control_Object_Name,
    Load_Control_Read_Property,
    Load_Control_Write_Property,
    Load_Control_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */ ,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
    {
    OBJECT_MULTISTATE_INPUT,
    Multistate_Input_Init,
    Multistate_Input_Count,
    Multistate_Input_Index_To_Instance,
    Multistate_Input_Valid_Instance,
    Multistate_Input_Object_Name,
    Multistate_Input_Read_Property,
    Multistate_Input_Write_Property,
    Multistate_Input_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1)
    Multistate_Input_Encode_Value_List,
    Multistate_Input_Change_Of_Value,
    Multistate_Input_Change_Of_Value_Clear,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B == 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1 )
    {
    OBJECT_MULTISTATE_OUTPUT,
    Multistate_Output_Init,
    Multistate_Output_Count,
    Multistate_Output_Index_To_Instance,
    Multistate_Output_Valid_Instance,
    Multistate_Output_Object_Name,
    Multistate_Output_Read_Property,
    Multistate_Output_Write_Property,
    Multistate_Output_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    Multistate_Output_Encode_Value_List,
    Multistate_Output_Change_Of_Value,
    Multistate_Output_Change_Of_Value_Clear,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
},
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
    {
    OBJECT_MULTISTATE_VALUE,
    Multistate_Value_Init,
    Multistate_Value_Count,
    Multistate_Value_Index_To_Instance,
    Multistate_Value_Valid_Instance,
    Multistate_Value_Object_Name,
    Multistate_Value_Read_Property,
    Multistate_Value_Write_Property,
    Multistate_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1)
    Multistate_Value_Encode_Value_List,
    Multistate_Value_Change_Of_Value,
    Multistate_Value_Change_Of_Value_Clear,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_TRENDLOG == 1 )
    {
    OBJECT_TRENDLOG,
    Trend_Log_Init,
    Trend_Log_Count,
    Trend_Log_Index_To_Instance,
    Trend_Log_Valid_Instance,
    Trend_Log_Object_Name,
    Trend_Log_Read_Property,
    Trend_Log_Write_Property,
    Trend_Log_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    TrendLogGetRRInfo,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B == 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_LIGHTING_OUTPUT == 1 )
#if (BACNET_PROTOCOL_REVISION >= 14)
    {
    OBJECT_LIGHTING_OUTPUT,
    Lighting_Output_Init,
    Lighting_Output_Count,
    Lighting_Output_Index_To_Instance,
    Lighting_Output_Valid_Instance,
    Lighting_Output_Object_Name,
    Lighting_Output_Read_Property,
    Lighting_Output_Write_Property,
    Lighting_Output_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif // BACNET_PROTOCOL_REVISION >= 14)
#endif

#if ( BACNET_USE_OBJECT_CHANNEL == 1 )
    {
    OBJECT_CHANNEL,
    Channel_Init,
    Channel_Count,
    Channel_Index_To_Instance,
    Channel_Valid_Instance,
    Channel_Object_Name,
    Channel_Read_Property,
    Channel_Write_Property,
    Channel_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if defined(BACFILE)
    {
    OBJECT_FILE,
    bacfile_init,
    bacfile_count,
    bacfile_index_to_instance,
    bacfile_valid_instance,
    bacfile_object_name,
    bacfile_read_property,
    bacfile_write_property,
    BACfile_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1 )
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( 0 )
    {
    OBJECT_OCTETSTRING_VALUE,
    OctetString_Value_Init,
    OctetString_Value_Count,
    OctetString_Value_Index_To_Instance,
    OctetString_Value_Valid_Instance,
    OctetString_Value_Object_Name,
    OctetString_Value_Read_Property,
    OctetString_Value_Write_Property,
    OctetString_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1 )
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_POSITIVE_INTEGER_VALUE == 1 )
    {
    OBJECT_POSITIVE_INTEGER_VALUE,
    PositiveInteger_Value_Init,
    PositiveInteger_Value_Count,
    PositiveInteger_Value_Index_To_Instance,
    PositiveInteger_Value_Valid_Instance,
    PositiveInteger_Value_Object_Name,
    PositiveInteger_Value_Read_Property,
    PositiveInteger_Value_Write_Property,
    PositiveInteger_Value_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B== 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_CALENDAR == 1 )
    {
    OBJECT_CALENDAR,
    Calendar_Init,
    Calendar_Count,
    Calendar_Index_To_Instance,
    Calendar_Valid_Instance,
    Calendar_Object_Name,
    Calendar_Read_Property,
    Calendar_Write_Property,
    Calendar_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    CalendarGetRRInfo,          // RR info
#endif
    NULL,                       // Object Iterator
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL,
    NULL,
#endif
#if (INTRINSIC_REPORTING_B== 1 )
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
    {
    OBJECT_SCHEDULE,
    Schedule_Init,
    Schedule_Count,
    Schedule_Index_To_Instance,
    Schedule_Valid_Instance,
    Schedule_Object_Name,
    Schedule_Read_Property,
    Schedule_Write_Property,
    Schedule_Property_Lists,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */,
#if ( BACNET_SVC_COV_B == 1 )
    NULL /* Value_Lists */,
    NULL /* COV */,
    NULL /* COV Clear */,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B == 1)
    NULL /* Intrinsic Reporting */
#endif
    },
#endif

    {
    MAX_BACNET_OBJECT_TYPE,
    NULL /* Init */,
    NULL /* Count */,
    NULL /* Index_To_Instance */,
    NULL /* Valid_Instance */,
    NULL /* Object_Name */,
    NULL /* Read_Property */,
    NULL /* Write_Property */,
    NULL /* Property_Lists */,
#if ( BACNET_SVC_RR_B == 1 )
    NULL /* ReadRangeInfo */,
#endif
    NULL /* Iterator */ ,
#if ( BACNET_SVC_COV_B == 1)
    NULL /* Value_Lists */ ,
    NULL /* COV */ ,
    NULL /* COV Clear */ ,
#endif
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
    NULL /* Add List Element */,
    NULL /* Remove List Element */,
#endif
#if (INTRINSIC_REPORTING_B == 1)
    NULL /* Intrinsic Reporting */
#endif
    }

};

/** Glue function to let the Device object, when called by a handler,
 * lookup which Object type needs to be invoked.
 * @ingroup ObjHelpers
 * @param Object_Type [in] The type of BACnet Object the handler wants to access.
 * @return Pointer to the group of object helper functions that implement this
 *         type of Object.
 */
static struct object_functions* Device_Objects_Find_Functions(
    BACNET_OBJECT_TYPE Object_Type) {
    struct object_functions* pObject = My_Object_Table;

    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        /* handle each object type */
        if (pObject->Object_Type == Object_Type) {
            return (pObject);
        }
        pObject++;
    }

    return (NULL);
}

/** Try to find a rr_info_function helper function for the requested object type.
 * @ingroup ObjIntf
 *
 * @param object_type [in] The type of BACnet Object the handler wants to access.
 * @return Pointer to the object helper function that implements the
 *         ReadRangeInfo function, Object_RR_Info, for this type of Object on
 *         success, else a NULL pointer if the type of Object isn't supported
 *         or doesn't have a ReadRangeInfo function.
 */
#if ( BACNET_SVC_RR_B == 1 )
rr_info_function Device_Objects_RR_Info(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_OBJECT_TYPE object_type) {
    struct object_functions* pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    return (pObject != NULL ? pObject->Object_RR_Info : NULL);
}
#endif

/** For a given object type, returns the special property list.
 * This function is used for ReadPropertyMultiple calls which want
 * just Required, just Optional, or All properties.
 * @ingroup ObjIntf
 *
 * @param object_type [in] The desired BACNET_OBJECT_TYPE whose properties
 *            are to be listed.
 * @param pPropertyList [out] Reference to the structure which will, on return,
 *            list, separately, the Required, Optional, and Proprietary object
 *            properties with their counts.
 */
void Device_Objects_Property_List(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    struct special_property_list_t* pPropertyList) {
    struct object_functions* pObject = NULL;

    (void)object_instance;
    pPropertyList->Required.pList = NULL;
    pPropertyList->Optional.pList = NULL;
    pPropertyList->Proprietary.pList = NULL;

    /* If we can find an entry for the required object type
     * and there is an Object_List_RPM fn ptr then call it
     * to populate the pointers to the individual list counters.
     */

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_RPM_List != NULL)) {
        pObject->Object_RPM_List(&pPropertyList->Required.pList,
            &pPropertyList->Optional.pList, &pPropertyList->Proprietary.pList);
    }

    /* Fetch the counts if available otherwise zero them */
    pPropertyList->Required.count =
        pPropertyList->Required.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Required.pList);

    pPropertyList->Optional.count =
        pPropertyList->Optional.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Optional.pList);

    pPropertyList->Proprietary.count =
        pPropertyList->Proprietary.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Proprietary.pList);

}


/** Commands a Device re-initialization, to a given state.
 * The request's password must match for the operation to succeed.
 * This implementation provides a framework, but doesn't
 * actually *DO* anything.
 * @note You could use a mix of states and passwords to multiple outcomes.
 * @note You probably want to restart *after* the simple ack has been sent
 *       from the return handler, so just set a local flag here.
 * @ingroup ObjIntf
 *
 * @param rd_data [in,out] The information from the RD request.
 *                         On failure, the error class and code will be set.
 * @return True if succeeds (password is correct), else False.
 */
bool Device_Reinitialize(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_REINITIALIZE_DEVICE_DATA* rd_data) {
    bool status = false;

    if (characterstring_ansi_same(&rd_data->password, "BACnet Testing")) {
        switch (rd_data->state) {
        case BACNET_REINIT_COLDSTART:
        case BACNET_REINIT_WARMSTART:
            dcc_set_status_duration(pDev, COMMUNICATION_ENABLE, 0);
            break;
        case BACNET_REINIT_STARTBACKUP:
            break;
        case BACNET_REINIT_ENDBACKUP:
            break;
        case BACNET_REINIT_STARTRESTORE:
            break;
        case BACNET_REINIT_ENDRESTORE:
            break;
        case BACNET_REINIT_ABORTRESTORE:
            break;
        default:
            break;
        }
        /* Note: you could use a mix of state
         and password to multiple things */
         /* note: you probably want to restart *after* the
          simple ack has been sent from the return handler
          so just set a flag from here */
        status = true;
    }
    else {
        rd_data->error_class = ERROR_CLASS_SECURITY;
        rd_data->error_code = ERROR_CODE_PASSWORD_FAILURE;
    }

    return status;
}

/* These three arrays are used by the ReadPropertyMultiple handler */
static const BACNET_PROPERTY_ID Device_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_SYSTEM_STATUS,
    PROP_VENDOR_NAME,
    PROP_VENDOR_IDENTIFIER,
    PROP_MODEL_NAME,
    PROP_FIRMWARE_REVISION,
    PROP_APPLICATION_SOFTWARE_VERSION,
    PROP_PROTOCOL_VERSION,
    PROP_PROTOCOL_REVISION,
    PROP_PROTOCOL_SERVICES_SUPPORTED,
    PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED,
    PROP_OBJECT_LIST,
    PROP_MAX_APDU_LENGTH_ACCEPTED,
    PROP_SEGMENTATION_SUPPORTED,
    PROP_APDU_TIMEOUT,
    PROP_NUMBER_OF_APDU_RETRIES,
    PROP_DEVICE_ADDRESS_BINDING,
    PROP_DATABASE_REVISION,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Device_Properties_Optional[] = {
#if (BACDL_MSTP == 1)
    PROP_MAX_MASTER,
    PROP_MAX_INFO_FRAMES,
#endif
    PROP_DESCRIPTION,
    PROP_LOCAL_TIME,
    PROP_UTC_OFFSET,
    PROP_LOCAL_DATE,
    PROP_DAYLIGHT_SAVINGS_STATUS,
    PROP_LOCATION,
    PROP_SERIAL_NUMBER,
    PROP_STRUCTURED_OBJECT_LIST,

# if(BACNET_SVC_COV_B == 1)
    PROP_ACTIVE_COV_SUBSCRIPTIONS,
#endif

#if (BACNET_TIME_MASTER == 1)
    PROP_TIME_SYNCHRONIZATION_RECIPIENTS,
    PROP_TIME_SYNCHRONIZATION_INTERVAL,
    PROP_ALIGN_INTERVALS,
    PROP_INTERVAL_OFFSET,
#endif


    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Device_Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};

void Device_Property_Lists(
    const BACNET_PROPERTY_ID** pRequired,
    const BACNET_PROPERTY_ID** pOptional,
    const BACNET_PROPERTY_ID** pProprietary) {
    if (pRequired) {
        *pRequired = Device_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Device_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Device_Properties_Proprietary;
    }
}

/* note: you really only need to define variables for
   properties that are writable or that may change.
   The properties that are constant can be hard coded
   into the read-property encoding. */

   // static uint32_t Object_Instance_Number = 260001;
   // static BACNET_CHARACTER_STRING My_Object_Name;
   // static BACNET_DEVICE_STATUS System_Status = STATUS_OPERATIONAL;
static const char* Vendor_Name = BACNET_VENDOR_NAME;
static uint16_t Vendor_Identifier = BACNET_VENDOR_ID;
// static char Model_Name[MAX_DEV_MOD_LEN + 1];
static char Application_Software_Version[MAX_DEV_VER_LEN + 1] = BACNET_VERSION_TEXT;
static char Location[MAX_DEV_LOC_LEN + 1] = "California";
/* static uint8_t Protocol_Version = 1; - constant, not settable */
/* static uint8_t Protocol_Revision = 4; - constant, not settable */
/* Protocol_Services_Supported - dynamically generated */
/* Protocol_Object_Types_Supported - in RP encoding */
/* Object_List - dynamically generated */
/* static BACNET_SEGMENTATION Segmentation_Supported = SEGMENTATION_NONE; */
/* static uint8_t Max_Segments_Accepted = 0; */
/* VT_Classes_Supported */
/* Active_VT_Sessions */
static BACNET_TIME Local_Time;  /* rely on OS, if there is one */
static BACNET_DATE Local_Date;  /* rely on OS, if there is one */

/* NOTE: BACnet UTC Offset is inverse of common practice.
   If your UTC offset is -5hours of GMT,
   then BACnet UTC offset is +5hours.
   BACnet UTC offset is expressed in minutes. */
static int32_t UTC_Offset = 8 * 60;

static bool Daylight_Savings_Status = false;    /* rely on OS */

static BACNET_CHARACTER_STRING SerialNumber ;

#if (BACNET_TIME_MASTER == 1)
static bool Align_Intervals;
static uint32_t Interval_Minutes;
static uint32_t Interval_Offset_Minutes;
/* Time_Synchronization_Recipients */
#endif
/* List_Of_Session_Keys */
/* Max_Master - rely on MS/TP subsystem, if there is one */
/* Max_Info_Frames - rely on MS/TP subsystem, if there is one */
/* Device_Address_Binding - required, but relies on binding cache */
// static uint32_t Database_Revision = 0;
/* Configuration_Files */
/* Last_Restore_Time */
/* Backup_Failure_Timeout */
/* Active_COV_Subscriptions */
/* Slave_Proxy_Enable */
/* Manual_Slave_Address_Binding */
/* Auto_Slave_Discovery */
/* Slave_Address_Binding */
/* Profile_Name */

unsigned Device_Count(
    DEVICE_OBJECT_DATA* pDev) {
    (void)pDev;
    return 1;
}

uint32_t Device_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index) {
    (void)index;
    return pDev->bacObj.objectInstance;
}

/* methods to manipulate the data */

/** Return the Object Instance number for our (single) Device Object.
 * This is a key function, widely invoked by the handler code, since
 * it provides "our" (ie, local) address.
 * @ingroup ObjIntf
 * @return The Instance number used in the BACNET_OBJECT_ID for the Device.
 */
uint32_t Device_Object_Instance_Number(
    const DEVICE_OBJECT_DATA* pDev) {
    return pDev->bacObj.objectInstance;
    //#ifdef BAC_ROUTING
    //    return Routed_Device_Object_Instance_Number();
    //#else
    //    return Object_Instance_Number;
    //#endif
}


// Only possible use is when BACnet Client wants to change device number. (and it does not (yet) persist, if changed)
static bool Device_Set_Object_Instance_Number(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_id) {
    bool status = true; /* return value */

    if (object_id <= BACNET_MAX_INSTANCE) {
        /* Make the change and update the database revision */
        pDev->bacObj.objectInstance = object_id;
        Device_Inc_Database_Revision(pDev);
    }
    else {
        status = false;
    }

    return status;
}

bool Device_Valid_Object_Instance_Number(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_id) {
    return (pDev->bacObj.objectInstance == object_id);
}


// this signature is required elsewhere
bool Device_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name) {
    bool status = false;

    if (object_instance == pDev->bacObj.objectInstance) {
        status = characterstring_copy(object_name, &pDev->bacObj.objectName);
    }

    return status;
}


bool Device_Set_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_CHARACTER_STRING* object_name) {
    bool status = false;        /*return value */

    if (!characterstring_same(&pDev->bacObj.objectName, object_name)) {
        /* Make the change and update the database revision */
        status = characterstring_copy(&pDev->bacObj.objectName, object_name);
        Device_Inc_Database_Revision(pDev);
    }

    return status;
}


bool Device_Object_Name_ANSI_Init(
    DEVICE_OBJECT_DATA* pDev,
    const char* value) {
    return characterstring_init_ansi(&pDev->bacObj.objectName, value);
}


//BACNET_DEVICE_STATUS Device_System_Status(
//    void)
//{
//    return System_Status;
//}



//int Device_Set_System_Status(
//    BACNET_DEVICE_STATUS status,
//    bool local)
//{
//    int result = 0;     /*return value - 0 = ok, -1 = bad value, -2 = not allowed */

//    /* We limit the options available depending on whether the source is
//     * internal or external. */
//    if (local) {
//        switch (status) {
//        case STATUS_OPERATIONAL:
//        case STATUS_OPERATIONAL_READ_ONLY:
//        case STATUS_DOWNLOAD_REQUIRED:
//        case STATUS_DOWNLOAD_IN_PROGRESS:
//        case STATUS_NON_OPERATIONAL:
//            System_Status = status;
//            break;

//        /* Don't support backup at present so don't allow setting */
//        case STATUS_BACKUP_IN_PROGRESS:
//            result = -2;
//            break;

//        default:
//            result = -1;
//            break;
//        }
//    } else {
//        switch (status) {
//        /* Allow these for the moment as a way to easily alter
//         * overall device operation. The lack of password protection
//         * or other authentication makes allowing writes to this
//         * property a risky facility to provide.
//         */
//        case STATUS_OPERATIONAL:
//        case STATUS_OPERATIONAL_READ_ONLY:
//        case STATUS_NON_OPERATIONAL:
//            System_Status = status;
//            break;

//        /* Don't allow outsider set this - it should probably
//         * be set if the device config is incomplete or
//         * corrupted or perhaps after some sort of operator
//         * wipe operation.
//         */
//        case STATUS_DOWNLOAD_REQUIRED:
//        /* Don't allow outsider set this - it should be set
//         * internally at the start of a multi packet download
//         * perhaps indirectly via PT or WF to a config file.
//         */
//        case STATUS_DOWNLOAD_IN_PROGRESS:
//        /* Don't support backup at present so don't allow setting */
//        case STATUS_BACKUP_IN_PROGRESS:
//            result = -2;
//            break;

//        default:
//            result = -1;
//            break;
//        }
//    }

//    return (result);
//}

const char* Device_Vendor_Name(
    void) {
    return Vendor_Name;
}

/** Returns the Vendor ID for this Device.
 * See the assignments at http://www.bacnet.org/VendorID/BACnet%20Vendor%20IDs.htm
 * @return The Vendor ID of this Device.
 */
uint16_t Device_Vendor_Identifier(
    void) {
    return Vendor_Identifier;
}

void Device_Set_Vendor_Identifier(
    uint16_t vendor_id) {
    Vendor_Identifier = vendor_id;
}

//const char *Device_Model_Name(
//    void)
//{
//    return Model_Name;
//}

//bool Device_Set_Model_Name(
//    const char *name )
//{
//    bool status = false;        /*return value */
//
//    if (strlen(name) < sizeof(Model_Name)) {
//        memmove(Model_Name, name, length);
//        Model_Name[MAX_DEV_MOD_LEN] = 0;
//        status = true;
//    }
//
//    return status;
//}

const char* Device_Firmware_Revision(
    void) {
    return BACnet_Version;
}

const char* Device_Application_Software_Version(
    void) {
    return Application_Software_Version;
}

bool Device_Set_Application_Software_Version(
    const char* name,
    uint16_t length) {
    bool status = false;        /*return value */

    if (length < sizeof(Application_Software_Version)) {
        memmove(Application_Software_Version, name, length);
        Application_Software_Version[length] = 0;
        status = true;
    }

    return status;
}


bool Device_Set_Description(
    DEVICE_OBJECT_DATA* pDev,
    const char* name) {
    return characterstring_init_ansi(&pDev->bacObj.description, name);
}


const char* Device_Location(
    void) {
    return Location;
}


bool Device_Set_Location(
    const char* name,
    uint16_t length) {
    bool status = false;        /*return value */

    if (length < sizeof(Location)) {
        memmove(Location, name, length);
        Location[length] = 0;
        status = true;
    }

    return status;
}

uint8_t Device_Protocol_Version(
    void) {
    return BACNET_PROTOCOL_VERSION;
}

uint8_t Device_Protocol_Revision(
    void) {
    return BACNET_PROTOCOL_REVISION;
}

BACNET_SEGMENTATION Device_Segmentation_Supported(
    void) {
    return SEGMENTATION_NONE;
}


uint32_t Device_Database_Revision(
    DEVICE_OBJECT_DATA* pDev) {
    return pDev->Database_Revision;
}


void Device_Set_Database_Revision(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t revision) {
    pDev->Database_Revision = revision;
}


/*
 * Shortcut for incrementing database revision as this is potentially
 * the most common operation if changing object names and ids is
 * implemented.
 */
void Device_Inc_Database_Revision(
    DEVICE_OBJECT_DATA* pDev) {
    pDev->Database_Revision++;
}

/** Get the total count of objects supported by this Device Object.
 * @note Since many network clients depend on the object list
 *       for discovery, it must be consistent!
 * @return The count of objects, for all supported Object types.
 */
unsigned Device_Object_List_Count(
    DEVICE_OBJECT_DATA* pDev) {
    unsigned count = 0; /* number of objects */
    struct object_functions* pObject = My_Object_Table;

    /* initialize the default return values */
    // pObject = Object_Table;
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Count) {
            count += pObject->Object_Count(pDev);
        }
        pObject++;
    }

    return count;
}

/** Lookup the Object at the given array index in the Device's Object List.
 * Even though we don't keep a single linear array of objects in the Device,
 * this method acts as though we do and works through a virtual, concatenated
 * array of all of our object type arrays.
 *
 * @param array_index [in] The desired array index (1 to N)
 * @param object_type [out] The object's type, if found.
 * @param instance [out] The object's instance number, if found.
 * @return True if found, else false.
 */
bool Device_Object_List_Identifier(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t array_index,
    BACNET_OBJECT_TYPE* object_type,
    uint32_t* instance) {
    bool status = false;
    uint32_t count = 0;
    uint32_t object_index = 0;
    uint32_t temp_index = 0;
    struct object_functions* pObject = My_Object_Table;

    /* array index zero is length - so invalid */
    if (array_index == 0) {
        return status;
    }
    object_index = array_index - 1;
    /* initialize the default return values */
    // pObject = Object_Table;
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Count) {
            object_index -= count;
            count = pObject->Object_Count(pDev);
            if (object_index < count) {
                /* Use the iterator function if available otherwise
                 * look for the index to instance to get the ID */
                if (pObject->Object_Iterator) {
                    /* First find the first object */
                    temp_index = pObject->Object_Iterator(pDev, ~(unsigned)0);
                    /* Then step through the objects to find the nth */
                    while (object_index != 0) {
                        temp_index = pObject->Object_Iterator(pDev, temp_index);
                        object_index--;
                    }
                    /* set the object_index up before falling through to next bit */
                    object_index = temp_index;
                }
                if (pObject->Object_Index_To_Instance) {
                    *object_type = pObject->Object_Type;
                    *instance = pObject->Object_Index_To_Instance(pDev, object_index);
                    status = true;
                    break;
                }
            }
        }
        pObject++;
    }

    return status;
}

/** Determine if we have an object with the given object_name.
 * If the object_type and object_instance pointers are not null,
 * and the lookup succeeds, they will be given the resulting values.
 * @param object_name [in] The desired Object Name to look for.
 * @param object_type [out] The BACNET_OBJECT_TYPE of the matching Object.
 * @param object_instance [out] The object instance number of the matching Object.
 * @return True on success or else False if not found.
 */
bool Device_Valid_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_CHARACTER_STRING* object_name1,
    BACNET_OBJECT_TYPE* object_type,
    uint32_t* object_instance) {
    bool found = false;
    BACNET_OBJECT_TYPE type;
    uint32_t instance;
    uint32_t max_objects = 0, i = 0;
    bool check_id = false;
    BACNET_CHARACTER_STRING object_name2;
    struct object_functions* pObject = NULL;

    max_objects = Device_Object_List_Count(pDev);
    for (i = 1; i <= max_objects; i++) {
        check_id = Device_Object_List_Identifier(pDev, i, &type, &instance);
        if (check_id) {
            pObject = Device_Objects_Find_Functions(type);
            if ((pObject != NULL) && (pObject->Object_Name != NULL) &&
                (pObject->Object_Name(pDev, instance, &object_name2) &&
                    characterstring_same(object_name1, &object_name2))) {
                found = true;
                if (object_type) {
                    *object_type = type;
                }
                if (object_instance) {
                    *object_instance = instance;
                }
                break;
            }
        }
    }

    return found;
}

/** Determine if we have an object of this type and instance number.
 * @param object_type [in] The desired BACNET_OBJECT_TYPE
 * @param object_instance [in] The object instance number to be looked up.
 * @return True if found, else False if no such Object in this device.
 */
bool Device_Valid_Object_Id(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance) {
    bool status = false;        /* return value */
    struct object_functions* pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_Valid_Instance != NULL)) {
        status = pObject->Object_Valid_Instance(pDev, object_instance);
    }

    return status;
}

/** Copy a child object's object_name value, given its ID.
 * @param object_type [in] The BACNET_OBJECT_TYPE of the child Object.
 * @param object_instance [in] The object instance number of the child Object.
 * @param object_name [out] The Object Name found for this child Object.
 * @return True on success or else False if not found.
 */
bool Device_Object_Name_Copy(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name) {
    struct object_functions* pObject = NULL;
    bool found = false;

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_Name != NULL)) {
        found = pObject->Object_Name(pDev, object_instance, object_name);
    }

    return found;
}


#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
time_t emulationOffset;
#endif


static long dstBiasSecs;

// Establish, at startup, timezone and DST, allowing modifications by BACnet clients later
void Device_Time_Init(
    void) {
#ifdef _MSC_VER
    // time_t utcTime = time(NULL);
    int daylight;
    long tzone;

    // struct tm *tblock = (struct tm *)localtime(&utcTime);
    _get_daylight(&daylight);
    Device_Daylight_Savings_Status_Set(daylight);

    _get_timezone(&tzone);
    Device_UTC_Offset_Set((int)(tzone / 60));

    // bias sign seems inverted according to docs (-3600 in summer. eh?)
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-dstbias
    _get_dstbias(&dstBiasSecs);
#endif
}


static void Update_Current_Time(
    void) {
    struct tm* tblock = NULL;
#if defined(_MSC_VER)
    time_t tTemp;
#else
    struct timeval tv;
#endif

    /*
    struct tm

    int    tm_sec   Seconds [0,60].
    int    tm_min   Minutes [0,59].
    int    tm_hour  Hour [0,23].
    int    tm_mday  Day of month [1,31].
    int    tm_mon   Month of year [0,11].
    int    tm_year  Years since 1900.
    int    tm_wday  Day of week [0,6] (Sunday =0).
    int    tm_yday  Day of year [0,365].
    int    tm_isdst Daylight Savings flag.
    */

#if defined(_MSC_VER)
    tTemp = time(NULL) + emulationOffset;
    tblock = (struct tm*)localtime(&tTemp);
#else
    if (gettimeofday(&tv, NULL) == 0) {
        tblock = (struct tm*)localtime((const time_t*)&tv.tv_sec);
    }
#endif

    if (tblock) {
        datetime_set_date(&Local_Date, (uint16_t)tblock->tm_year + 1900,
            (uint8_t)tblock->tm_mon + 1, (uint8_t)tblock->tm_mday);
#if !defined(_MSC_VER)
        datetime_set_time(&Local_Time, (uint8_t)tblock->tm_hour,
            (uint8_t)tblock->tm_min, (uint8_t)tblock->tm_sec,
            (uint8_t)(tv.tv_usec / 10000));
#else
        datetime_set_time(&Local_Time, (uint8_t)tblock->tm_hour,
            (uint8_t)tblock->tm_min, (uint8_t)tblock->tm_sec, 0);
#endif
        if (tblock->tm_isdst) {
            Daylight_Savings_Status = true;
        }
        else {
            Daylight_Savings_Status = false;
        }

#if !defined(_MSC_VER)
        /* note: timezone is declared in <time.h> stdlib. */
        UTC_Offset = timezone / 60;
#else
        TIME_ZONE_INFORMATION tziOld;
        GetTimeZoneInformation(&tziOld);
        UTC_Offset = tziOld.Bias;
#endif

    }
    else {
        datetime_date_wildcard_set(&Local_Date);
        datetime_time_wildcard_set(&Local_Time);
        Daylight_Savings_Status = false;
    }
}


void Device_getCurrentDateTime(
    BACNET_DATE_TIME* DateTime) {
    Update_Current_Time();

    DateTime->date = Local_Date;
    DateTime->time = Local_Time;
}

int32_t Device_UTC_Offset(void) {
    Update_Current_Time();

    return UTC_Offset;
}

void Device_UTC_Offset_Set(int offset) {
    // what about real-time-clock on PC?
    UTC_Offset = offset;
}

bool Device_Daylight_Savings_Status(void) {
    return Daylight_Savings_Status;
}

void Device_Daylight_Savings_Status_Set(int status) {
    Daylight_Savings_Status = (status != 0) ? true : false;
}


#if (BACNET_TIME_MASTER == 1)
/**
 * Sets the time sync interval in minutes
 *
 * @param flag
 * This property, of type BOOLEAN, specifies whether (TRUE)
 * or not (FALSE) clock-aligned periodic time synchronization is
 * enabled. If periodic time synchronization is enabled and the
 * time synchronization interval is a factor of (divides without
 * remainder) an hour or day, then the beginning of the period
 * specified for time synchronization shall be aligned to the hour or
 * day, respectively. If this property is present, it shall be writable.
 */
bool Device_Align_Intervals_Set(bool flag) {
    Align_Intervals = flag;

    return true;
}

bool Device_Align_Intervals(void) {
    return Align_Intervals;
}

/**
 * Sets the time sync interval in minutes
 *
 * @param minutes
 * This property, of type Unsigned, specifies the periodic
 * interval in minutes at which TimeSynchronization and
 * UTCTimeSynchronization requests shall be sent. If this
 * property has a value of zero, then periodic time synchronization is
 * disabled. If this property is present, it shall be writable.
 */
bool Device_Time_Sync_Interval_Set(uint32_t minutes) {
    Interval_Minutes = minutes;

    return true;
}

uint32_t Device_Time_Sync_Interval(void) {
    return Interval_Minutes;
}

/**
 * Sets the time sync interval offset value.
 *
 * @param minutes
 * This property, of type Unsigned, specifies the offset in
 * minutes from the beginning of the period specified for time
 * synchronization until the actual time synchronization requests
 * are sent. The offset used shall be the value of Interval_Offset
 * modulo the value of Time_Synchronization_Interval;
 * e.g., if Interval_Offset has the value 31 and
 * Time_Synchronization_Interval is 30, the offset used shall be 1.
 * Interval_Offset shall have no effect if Align_Intervals is
 * FALSE. If this property is present, it shall be writable.
 */
bool Device_Interval_Offset_Set(uint32_t minutes) {
    Interval_Offset_Minutes = minutes;

    return true;
}

uint32_t Device_Interval_Offset(void) {
    return Interval_Offset_Minutes;
}
#endif


int Device_encode_proprietary_comm_stats(
    uint8_t* apdu,
    const uint32_t value) {
    int len = 0;

    len += encode_application_unsigned(&apdu[len], value);
    len += encode_application_unsigned(&apdu[len], 2);
    len += encode_application_unsigned(&apdu[len], 3);
    len += encode_application_unsigned(&apdu[len], 4);
    len += encode_application_unsigned(&apdu[len], 5);
    len += encode_application_unsigned(&apdu[len], 6);
    len += encode_application_unsigned(&apdu[len], 7);
    len += encode_application_unsigned(&apdu[len], 8);
    len += encode_application_unsigned(&apdu[len], 9);
    len += encode_application_unsigned(&apdu[len], 10);

    return len;
}


/* return the length of the apdu encoded or BACNET_STATUS_ERROR for error or
   BACNET_STATUS_ABORT for abort message */
int Device_Read_Property_Local(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata) {
    int apdu_len;   /* return value */
    int len;        /* apdu len intermediate value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint32_t i = 0;
    BACNET_OBJECT_TYPE object_type;
    uint32_t instance;
    uint32_t count = 0;
    uint8_t* apdu;
    struct object_functions* pObject = My_Object_Table;
    bool found = false;
    uint16_t apdu_max;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ABORT;
    }

    apdu = rpdata->application_data;
    apdu_max = rpdata->application_data_len;

    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0], OBJECT_DEVICE,
                pDev->bacObj.objectInstance);
        break;

    case PROP_OBJECT_NAME:
        apdu_len =
            encode_application_character_string(&apdu[0],
                &pDev->bacObj.objectName);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len = encode_application_enumerated(&apdu[0], OBJECT_DEVICE);
        break;

    case PROP_DESCRIPTION:
        apdu_len =
            encode_application_character_string(&apdu[0], &pDev->bacObj.description);
        break;

    case PROP_SYSTEM_STATUS:
        apdu_len = encode_application_enumerated(&apdu[0], pDev->System_Status);
        break;

    // object specific

    case PROP_SERIAL_NUMBER:
        apdu_len =
            encode_application_character_string(&apdu[0], &SerialNumber );
        break;

    case PROP_VENDOR_NAME:
        characterstring_init_ansi(&char_string, Vendor_Name);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_VENDOR_IDENTIFIER:
        apdu_len =
            encode_application_unsigned(&apdu[0], Vendor_Identifier);
        break;

    case PROP_MODEL_NAME:
        apdu_len =
            encode_application_character_string(&apdu[0], &pDev->Model_Name);
        break;

    case PROP_FIRMWARE_REVISION:
        characterstring_init_ansi(&char_string, BACnet_Version);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_APPLICATION_SOFTWARE_VERSION:
        characterstring_init_ansi(&char_string,
            Application_Software_Version);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_LOCATION:
        characterstring_init_ansi(&char_string, Location);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_LOCAL_TIME:
        Update_Current_Time();
        apdu_len = encode_application_time(&apdu[0], &Local_Time);
        break;

    case PROP_UTC_OFFSET:
        // Update_Current_Time();
        apdu_len = encode_application_signed(&apdu[0], UTC_Offset);
        break;

    case PROP_LOCAL_DATE:
        Update_Current_Time();
        apdu_len = encode_application_date(&apdu[0], &Local_Date);
        break;

    case PROP_DAYLIGHT_SAVINGS_STATUS:
        // Update_Current_Time();
        // currently, we only allow manual alteration of DST status. It is set on startup, but how will we control
        // things if user overrides clock??
        apdu_len =
            encode_application_boolean(&apdu[0], Daylight_Savings_Status);
        break;

    case PROP_PROTOCOL_VERSION:
        apdu_len =
            encode_application_unsigned(&apdu[0],
                Device_Protocol_Version());
        break;

    case PROP_PROTOCOL_REVISION:
        apdu_len =
            encode_application_unsigned(&apdu[0],
                Device_Protocol_Revision());
        break;

    case PROP_PROTOCOL_SERVICES_SUPPORTED:
        /* Note: list of services that are executed, not initiated. */
        bitstring_init(&bit_string);
        for (i = 0; i < MAX_BACNET_SERVICES_SUPPORTED; i++) {
            /* automatic lookup based on handlers set */
            bitstring_set_bit(&bit_string, (uint8_t)i,
                apdu_service_supported((BACNET_SERVICES_SUPPORTED)i));
        }
        apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
        break;

    case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
        /* Note: this is the list of objects that can be in this device,
           not a list of objects that this device can access */
        bitstring_init(&bit_string);
        for (i = 0; i < MAX_ASHRAE_OBJECT_TYPE; i++) {
            /* initialize all the object types to not-supported */
            bitstring_set_bit(&bit_string, (uint8_t)i, false);
        }
        /* set the object types with objects to supported */

        // pObject = Object_Table;
        while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
            if ((pObject->Object_Count) && (pObject->Object_Count(pDev) > 0)) {
                bitstring_set_bit(&bit_string, pObject->Object_Type, true);
            }
            pObject++;
        }
        apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
        break;

    case PROP_OBJECT_LIST:
        count = Device_Object_List_Count(pDev);
        /* Array element zero is the number of objects in the list */
        if (rpdata->array_index == 0) {
            apdu_len = encode_application_unsigned(&apdu[0], count);
        }
        /* if no index was specified, then try to encode the entire list */
        /* into one packet.  Note that more than likely you will have */
        /* to return an error if the number of encoded objects exceeds */
        /* your maximum APDU size. */
        else if (rpdata->array_index == BACNET_ARRAY_ALL) {
            apdu_len = 0;
            for (i = 1; i <= count; i++) {
                found =
                    Device_Object_List_Identifier(pDev, i, &object_type,
                        &instance);
                if (found) {
                    len =
                        encode_application_object_id(&apdu[apdu_len],
                            object_type, instance);
                    apdu_len += len;
                    /* assume next one is the same size as this one */
                    /* can we all fit into the APDU? Don't check for last entry */
                    if ((i != count) && (apdu_len + len) >= apdu_max) {
                        /* Abort response */
                        rpdata->error_code =
                            ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                        apdu_len = BACNET_STATUS_ABORT;
                        break;
                    }
                }
                else {
                    /* error: internal error? */
                    rpdata->error_class = ERROR_CLASS_SERVICES;
                    rpdata->error_code = ERROR_CODE_OTHER;
                    apdu_len = BACNET_STATUS_ERROR;
                    break;
                }
            }
        }
        else {
            found =
                Device_Object_List_Identifier(pDev, rpdata->array_index,
                    &object_type, &instance);
            if (found) {
                apdu_len =
                    encode_application_object_id(&apdu[0], object_type,
                        instance);
            }
            else {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = BACNET_STATUS_ERROR;
            }
        }
        break;

    case PROP_STRUCTURED_OBJECT_LIST:
        /* Array element zero is the number of objects in the list */
        if (rpdata->array_index == 0) {
            apdu_len = encode_application_unsigned(&apdu[0], count);
        }
        else if (rpdata->array_index == BACNET_ARRAY_ALL) {
            // always empty - nothing to encode.
            apdu_len = 0;
        }
        else {
            // always empty, trying to read any item must fail
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
            apdu_len = BACNET_STATUS_ERROR;
        }
        break;

    case PROP_MAX_APDU_LENGTH_ACCEPTED:
        apdu_len = encode_application_unsigned(&apdu[0], MAX_APDU_IP);
        break;

    case PROP_SEGMENTATION_SUPPORTED:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                Device_Segmentation_Supported());
        break;

    case PROP_APDU_TIMEOUT:
        apdu_len = encode_application_unsigned(&apdu[0], apdu_timeout());
        break;

    case PROP_NUMBER_OF_APDU_RETRIES:
        apdu_len = encode_application_unsigned(&apdu[0], apdu_retries());
        break;

    case PROP_DEVICE_ADDRESS_BINDING:
        // this is a required property, so we have to process it, even if we do not have any address bindings
#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1)
        apdu_len = address_list_encode(pDev->datalink, &apdu[0], apdu_max);
#else 
        apdu_len = 0; 
#endif
        break;

    case PROP_DATABASE_REVISION:
        apdu_len =
            encode_application_unsigned(&apdu[0], pDev->Database_Revision);
        break;

#if (BACDL_MSTP == 1)
    case PROP_MAX_INFO_FRAMES:
        //apdu_len =
        //    encode_application_unsigned(&apdu[0],
        //        dlmstp_max_info_frames());
        break;
    case PROP_MAX_MASTER:
        //apdu_len =
        //    encode_application_unsigned(&apdu[0], dlmstp_max_master());
        break;
#endif


#if (BACNET_TIME_MASTER == 1)
    case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
        apdu_len = handler_timesync_encode_recipients(&apdu[0], MAX_LPDU_IP);
        if (apdu_len < 0) {
            rpdata->error_code =
                ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
            apdu_len = BACNET_STATUS_ABORT;
        }
        break;
    case PROP_TIME_SYNCHRONIZATION_INTERVAL:
        apdu_len = encode_application_unsigned(&apdu[0],
            Device_Time_Sync_Interval());
        break;
    case PROP_ALIGN_INTERVALS:
        apdu_len =
            encode_application_boolean(&apdu[0],
                Device_Align_Intervals());
        break;
    case PROP_INTERVAL_OFFSET:
        apdu_len = encode_application_unsigned(&apdu[0],
            Device_Interval_Offset());
        break;
#endif

#if ( BACNET_SVC_COV_B == 1 )
    case PROP_ACTIVE_COV_SUBSCRIPTIONS:
        apdu_len = handler_cov_encode_subscriptions(pDev, &apdu[0], apdu_max);
        break;
#endif // BAC_COV

    case PROP_MAX_SEGMENTS_ACCEPTED:
        //    case PROP_TIME_SYNCHRONIZATION_INTERVAL:
        //    case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
    case PROP_PROFILE_NAME:
        //    case PROP_INTERVAL_OFFSET:
        //    case PROP_ALIGN_INTERVALS:
    case PROP_TIME_OF_DEVICE_RESTART:
    case PROP_VT_CLASSES_SUPPORTED:
    case PROP_ACTIVE_VT_SESSIONS:
    case PROP_UTC_TIME_SYNCHRONIZATION_RECIPIENTS:
    case PROP_LAST_RESTART_REASON:
#if ( BACNET_SVC_COV_B == 0 )
    case PROP_ACTIVE_COV_SUBSCRIPTIONS:
#endif
    case PROP_SLAVE_PROXY_ENABLE:
    case PROP_MANUAL_SLAVE_ADDRESS_BINDING:
    case PROP_AUTO_SLAVE_DISCOVERY:
    case PROP_BACKUP_AND_RESTORE_STATE:
    case PROP_SLAVE_ADDRESS_BINDING:
    case PROP_BACKUP_FAILURE_TIMEOUT:
    case PROP_BACKUP_PREPARATION_TIME:
#if ( BACDL_MSTP == 0)
    case PROP_MAX_INFO_FRAMES:
    case PROP_MAX_MASTER:
#endif
    case PROP_CONFIGURATION_FILES:
    case PROP_APDU_SEGMENT_TIMEOUT:
    case PROP_RESTORE_COMPLETION_TIME:
    case PROP_RESTORE_PREPARATION_TIME:
    case PROP_LAST_RESTORE_TIME:
    case PROP_RESTART_NOTIFICATION_RECIPIENTS:
        // these are optional properties, so dont dump an error for them just because we are 'ignoring' the request by returning 'unknown property'
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_UNKNOWN_PROPERTY;
        break;

    default:
        dbMessage(DBD_ALL, DB_NORMAL_TRAFFIC, "Unknown Property %s", bactext_property_name(rpdata->object_property));
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }

    /*  only array properties can have array options */
    if ((apdu_len >= 0) &&
        (rpdata->object_property != PROP_OBJECT_LIST) &&
        (rpdata->object_property != PROP_STRUCTURED_OBJECT_LIST) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}


#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
bool Device_Add_List_Element(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_LIST_MANIPULATION_DATA* lmdata) {
    bool status = false;
    struct object_functions* pObject = NULL;

    /* initialize the default return values */
    lmdata->error_class = ERROR_CLASS_PROPERTY;
    lmdata->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
    pObject = Device_Objects_Find_Functions(lmdata->object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(pDev, lmdata->object_instance)) {
            if (pObject->Object_Add_List_Element) {
                status = pObject->Object_Add_List_Element(pDev, lmdata);
            }
        }
    }

    return status;
}

bool Device_Remove_List_Element(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_LIST_MANIPULATION_DATA* lmdata) {
    bool status = false;
    struct object_functions* pObject = NULL;

    /* initialize the default return values */
    lmdata->error_class = ERROR_CLASS_PROPERTY;
    lmdata->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
    pObject = Device_Objects_Find_Functions(lmdata->object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(pDev, lmdata->object_instance)) {
            if (pObject->Object_Remove_List_Element) {
                status = pObject->Object_Remove_List_Element(pDev, lmdata);
            }
        }
    }

    return status;
}
#endif // #if ( BACNET_SVC_LIST_MANIPULATION_B == 1)


/** Looks up the requested Object and Property, and encodes its Value in an APDU.
 * @ingroup ObjIntf
 * If the Object or Property can't be found, sets the error class and code.
 *
 * @param rpdata [in,out] Structure with the desired Object and Property info
 *                 on entry, and APDU message on return.
 * @return The length of the APDU on success, else BACNET_STATUS_ERROR
 */
int Device_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata) {
    int apdu_len = BACNET_STATUS_ERROR;
    struct object_functions* pObject = NULL;
#if (BACNET_PROTOCOL_REVISION >= 14)
    struct special_property_list_t property_list;
#endif

    /* initialize the default return values */
    rpdata->error_class = ERROR_CLASS_OBJECT;
    rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    pObject = Device_Objects_Find_Functions(rpdata->object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance)
        {
            if (pObject->Object_Valid_Instance(pDev, rpdata->object_instance))
            {
                if (pObject->Object_Read_Property)
                {
#if (BACNET_PROTOCOL_REVISION >= 14)
                    if ((int)rpdata->object_property == PROP_PROPERTY_LIST)
                    {
                        Device_Objects_Property_List(
                            rpdata->object_type,
                            rpdata->object_instance,
                            &property_list);
                        apdu_len = property_list_encode(
                            rpdata,
                            property_list.Required.pList,
                            property_list.Optional.pList,
                            property_list.Proprietary.pList);
                    }
                    else
#endif
                    {
                        apdu_len = pObject->Object_Read_Property(pDev, rpdata);
                    }
                }
            }
#ifdef AUTOCREATE_SITE
            else
            {
                char tname[100];
                char tdesc[100];
                sprintf(tname, "%s-%d", bactext_object_type_acronym_by_enum(rpdata->object_type),  rpdata->object_instance);
                sprintf(tdesc, "%s:%-07d Autocreated Object on Read", bactext_object_type_acronym_by_enum(rpdata->object_type), rpdata->object_instance);
                printf("   Dev:%-07d - %s\n", pDev->bacObj.objectInstance, tdesc);
                BACapi_Create_Object(pDev->bacObj.objectInstance, rpdata->object_type, rpdata->object_instance, tname, tdesc);
                apdu_len = pObject->Object_Read_Property(pDev, rpdata);
            }
#endif
        }
    }
    else
    {
        printf("Object type %s not found.", bactext_object_type_acronym_by_enum(rpdata->object_type));
    }

    return apdu_len;
}

/* returns true if successful */
bool Device_Write_Property_Local(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data) {
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance = 0;

    /* decode some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
            wp_data->application_data_len, &value);
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    if ((wp_data->object_property != PROP_OBJECT_LIST) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    /* FIXME: len < application_data_len: more data? */

    switch (wp_data->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_OBJECT_ID,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if ((value.type.Object_Id.type == OBJECT_DEVICE) &&
                (Device_Set_Object_Instance_Number(pDev, value.type.Object_Id.instance))) {
                /* FIXME: we could send an I-Am broadcast to let the world know */
            }
            else {
                status = false;
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;

    case PROP_NUMBER_OF_APDU_RETRIES:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            /* FIXME: bounds check? */
            apdu_retries_set((uint8_t)value.type.Unsigned_Int);
        }
        break;

    case PROP_APDU_TIMEOUT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            /* FIXME: bounds check? */
            apdu_timeout_set((uint16_t)value.type.Unsigned_Int);
        }
        break;
        // EKH: Makes no sense that system status is writable
        //case PROP_SYSTEM_STATUS:
        //    status =
        //        WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
        //                          &wp_data->error_class, &wp_data->error_code);
        //    if (status) {
        //        temp = Device_Set_System_Status((BACNET_DEVICE_STATUS)
        //                                        value.type.Enumerated, false);
        //        if (temp != 0) {
        //            status = false;
        //            wp_data->error_class = ERROR_CLASS_PROPERTY;
        //            if (temp == -1) {
        //                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        //            } else {
        //                wp_data->error_code =
        //                    ERROR_CODE_OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED;
        //            }
        //        }
        //    }
        //    break;

    case PROP_OBJECT_NAME:
        status =
            WPValidateString(&value,
                characterstring_capacity(&pDev->bacObj.objectName), false,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            /* All the object names in a device must be unique */
            if (Device_Valid_Object_Name(pDev, &value.type.Character_String,
                &object_type, &object_instance)) {
                if ((object_type == wp_data->object_type) &&
                    (object_instance == wp_data->object_instance)) {
                    /* writing same name to same object */
                    status = true;
                }
                else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_DUPLICATE_NAME;
                }
            }
            else {
                Device_Set_Object_Name(pDev, &value.type.Character_String);
                pDev->Database_Revision++;
            }
        }
        break;

    case PROP_LOCATION:
        status =
            WPValidateString(&value, MAX_DEV_LOC_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Device_Set_Location(characterstring_value(&value.
                type.Character_String),
                characterstring_length(&value.type.Character_String));
        }
        break;

    case PROP_DESCRIPTION:
        status =
            WPValidateString(&value, MAX_DEV_DESC_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Device_Set_Description(
                pDev,
                value.type.Character_String.value);
        }
        break;

        // Makes no sense to be writeable
        //case PROP_MODEL_NAME:
        //    status =
        //        WPValidateString(&value, MAX_DEV_MOD_LEN, true,
        //            &wp_data->error_class, &wp_data->error_code);
        //    if (status) {
        //        Device_Set_Model_Name(characterstring_value(&value.
        //            type.Character_String),
        //            characterstring_length(&value.type.Character_String));
        //    }
        //    break;

#if (BACNET_TIME_MASTER == 1)
    case PROP_TIME_SYNCHRONIZATION_INTERVAL:
        if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
            if (value.type.Unsigned_Int < 65535) {
                minutes = value.type.Unsigned_Int;
                Device_Time_Sync_Interval_Set(minutes);
                status = true;
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
        break;
    case PROP_ALIGN_INTERVALS:
        if (value.tag == BACNET_APPLICATION_TAG_BOOLEAN) {
            Device_Align_Intervals_Set(value.type.Boolean);
            status = true;
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
        break;
    case PROP_INTERVAL_OFFSET:
        if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
            if (value.type.Unsigned_Int < 65535) {
                minutes = value.type.Unsigned_Int;
                Device_Interval_Offset_Set(minutes);
                status = true;
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
        break;
#else
    case PROP_TIME_SYNCHRONIZATION_INTERVAL:
    case PROP_ALIGN_INTERVALS:
    case PROP_INTERVAL_OFFSET:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
#endif

    case PROP_UTC_OFFSET:
#if 0
        // for now, punt on this, modifying UTC_OFFSET on a pc with RTC means localtime, utc on pc have to be changed too. too complicated to think about for now, making Read only
        if (value.tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
            if ((value.type.Signed_Int < (12 * 60)) &&
                (value.type.Signed_Int > (-12 * 60))) {
                Device_UTC_Offset_Set(value.type.Signed_Int);
                status = true;
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
#else
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
#endif
        break;

#if (BACDL_MSTP==1)
    case PROP_MAX_INFO_FRAMES:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (value.type.Unsigned_Int <= 255) {
                //dlmstp_set_max_info_frames(
                //    (uint8_t)value.
                //    type.Unsigned_Int);
            }
            else {
                status = false;
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;

    case PROP_MAX_MASTER:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if ((value.type.Unsigned_Int > 0) &&
                (value.type.Unsigned_Int <= 127)) {
                //dlmstp_set_max_master(
                //    (uint8_t)value.type.Unsigned_Int);
            }
            else {
                status = false;
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;
#else
    case PROP_MAX_INFO_FRAMES:
    case PROP_MAX_MASTER:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
#endif
    case PROP_ACTIVE_COV_SUBSCRIPTIONS:
    case PROP_APPLICATION_SOFTWARE_VERSION:
    case PROP_DATABASE_REVISION:
    case PROP_DAYLIGHT_SAVINGS_STATUS:
    case PROP_DEVICE_ADDRESS_BINDING:
    case PROP_FIRMWARE_REVISION:
    case PROP_OBJECT_TYPE:
    case PROP_SERIAL_NUMBER:
    case PROP_LOCAL_TIME:
    case PROP_LOCAL_DATE:
    case PROP_MAX_APDU_LENGTH_ACCEPTED:
    case PROP_MODEL_NAME:
    case PROP_OBJECT_LIST:
    case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
    case PROP_PROTOCOL_REVISION:
    case PROP_PROTOCOL_SERVICES_SUPPORTED:
    case PROP_PROTOCOL_VERSION:
    case PROP_SEGMENTATION_SUPPORTED:
    case PROP_STRUCTURED_OBJECT_LIST:
    case PROP_SYSTEM_STATUS:
    case PROP_VENDOR_IDENTIFIER:
    case PROP_VENDOR_NAME:
#if (BACNET_TIME_MASTER == 1)
    case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
#endif
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        break;

    default:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
    }

    return status;
}

/** Looks up the requested Object and Property, and set the new Value in it,
 *  if allowed.
 * If the Object or Property can't be found, sets the error class and code.
 * @ingroup ObjIntf
 *
 * @param wp_data [in,out] Structure with the desired Object and Property info
 *              and new Value on entry, and APDU message on return.
 * @return True on success, else False if there is an error.
 */
bool Device_Write_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data) {
    bool status = false;        /* Ever the pessamist! */
    struct object_functions* pObject = NULL;

    /* initialize the default return values */
    wp_data->error_class = ERROR_CLASS_OBJECT;
    wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    pObject = Device_Objects_Find_Functions(wp_data->object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance)
        {
            if (pObject->Object_Valid_Instance(pDev, wp_data->object_instance))
            {
                if (pObject->Object_Write_Property)
                {
#if (BACNET_PROTOCOL_REVISION >= 14)
                    if (wp_data->object_property == PROP_PROPERTY_LIST)
                    {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                    }
                    else
#endif
                    {
                        status = pObject->Object_Write_Property(pDev, wp_data);
                    }
                }
                else
                {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                }
            }
            else
            {
#ifdef AUTOCREATE_SITE
                char tname[100];
                char tdesc[100];
                sprintf(tname, "%s-%-07d", bactext_object_type_acronym_by_enum(wp_data->object_type), wp_data->object_instance);
                sprintf(tdesc, "%s:%-07d Autocreated Object on Write", bactext_object_type_acronym_by_enum(wp_data->object_type), wp_data->object_instance);
                printf("   Dev:%-07d - %s\n", pDev->bacObj.objectInstance, tdesc);
                BACapi_Create_Object(pDev->bacObj.objectInstance, wp_data->object_type, wp_data->object_instance, tname, tdesc);
                status = pObject->Object_Write_Property(pDev, wp_data);
#else
                wp_data->error_class = ERROR_CLASS_OBJECT;
                wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
#endif
            }
        }
    }
    else {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    }

    return (status);
}

#if ( BACNET_SVC_COV_B == 1 )
/** Looks up the requested Object, and fills the Property Value list.
 * If the Object or Property can't be found, returns false.
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance number to be looked up.
 * @param [out] The value list
 * @return True if the object instance supports this feature and value changed.
 */
bool Device_Encode_Value_List(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE* value_list) {
    bool status = false;        /* Ever the pessamist! */
    struct object_functions* pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(pDev, object_instance)) {
            if (pObject->Object_Value_List) {
                status =
                    pObject->Object_Value_List(pDev, object_instance, value_list);
            }
        }
    }

    return (status);
}

/** Checks the COV flag in the requested Object
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance to be looked up.
 * @return True if the COV flag is set
 */
bool Device_COV(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance) {
    bool status = false;        /* Ever the pessamist! */
    struct object_functions* pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&

            // todo 3 - on large systems, this sequence of validating instance, then calling Object_COV forces two
            // searches of the expensive object list... we can find a better way.
            // also, that search of the object list -> currently it is a search along a linked list. It should at least
            // be a binary search of a (reallocated) array of objects or object pointers....

            pObject->Object_Valid_Instance(pDev, object_instance)) {
            if (pObject->Object_COV) {
                status = pObject->Object_COV(pDev, object_instance);
            }
        }
    }

    return (status);
}

/** Clears the COV flag in the requested Object
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance to be looked up.
 */
void Device_COV_Clear(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance) {
    struct object_functions* pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(pDev, object_instance)) {
            if (pObject->Object_COV_Clear) {
                pObject->Object_COV_Clear(pDev, object_instance);
            }
        }
    }
}
#endif // BACNET_SVC_COV_B


#if (INTRINSIC_REPORTING_B == 1)
void Device_local_reporting(
    DEVICE_OBJECT_DATA* pDev
) {
    struct object_functions* pObject;
    uint32_t objects_count;
    uint32_t object_instance;
    BACNET_OBJECT_TYPE object_type;
    uint32_t idx;

    objects_count = Device_Object_List_Count(pDev);

    /* loop for all objects */
    // other versions have <= here
    for (idx = 1; idx < objects_count; idx++) {
        Device_Object_List_Identifier(pDev, idx, &object_type, &object_instance);

        pObject = Device_Objects_Find_Functions(object_type);
        if (pObject != NULL) {
            if (pObject->Object_Valid_Instance &&
                pObject->Object_Valid_Instance(pDev, object_instance)) {
                if (pObject->Object_Intrinsic_Reporting) {
                    pObject->Object_Intrinsic_Reporting(pDev, object_instance);
                }
            }
        }
    }
}
#endif

#if ( BACNET_SVC_COV_B == 1)
/** Looks up the requested Object to see if the functionality is supported.
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @return True if the object instance supports this feature.
 */
bool Device_Value_List_Supported(
    BACNET_OBJECT_TYPE object_type) {
    bool status = false;        /* Ever the pessamist! */
    struct object_functions* pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Value_List) {
            status = true;
        }
    }

    return (status);
}
#endif

#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
int Device_decode_address_binding(
    uint8_t* application_data,
    uint32_t application_data_len,
    BACNET_OBJECT_ID* object_id,
    BACNET_PATH* address) {
    BACNET_APPLICATION_DATA_VALUE value;
    int len = 0;
    int pos = 0;

    // object id
    len = bacapp_decode_application_data(&application_data[pos], application_data_len, &value);
    if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_OBJECT_ID)) {
        return -1;
    }
    *object_id = value.type.Object_Id;
    pos += len;
    // network id
    len = bacapp_decode_application_data(&application_data[pos], application_data_len, &value);
    if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT)) {
        return -1;
    }
    address->glAdr.net = value.type.Unsigned_Int;
    pos += len;
    // mac address
    len = bacapp_decode_application_data(&application_data[pos], application_data_len, &value);
    if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_OCTET_STRING)) {
        return -1;
    }
    pos += len;
    /* store value */
    if (address->glAdr.net == 0) {
        memcpy(address->glAdr.mac.bytes, value.type.Octet_String.value, value.type.Octet_String.length);
        address->glAdr.mac.len = value.type.Octet_String.length;
    }
    else {
        memcpy(address->localMac.bytes, value.type.Octet_String.value, value.type.Octet_String.length);
        address->localMac.len = value.type.Octet_String.length;
    }

    return pos;
}
#endif

#if ( BACNET_SVC_LIST_MANIPULATION_B == 1)
bool Device_Add_List_Element_Local(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_LIST_MANIPULATION_DATA* lmdata) {
    BACNET_OBJECT_ID object_id;
    BACNET_ROUTE route;
    int end = lmdata->application_max_data_len;
    int pos = 0;
    int len = 0;

    switch (lmdata->object_property) {
    case PROP_DEVICE_ADDRESS_BINDING:
        /* validate list elements */
        while (pos < end) {
            len = Device_decode_address_binding(
                &lmdata->application_data[pos], lmdata->application_max_data_len - pos, &object_id, &route.bacnetPath);
            if (len < 0) {
                lmdata->error_class = ERROR_CLASS_PROPERTY;
                lmdata->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                return false;
            }
            lmdata->first_failed_element++;
            pos += len;
        }
        pos = 0;
        /* insert the list elements into the address cache */
        while (pos < end) {
            len = Device_decode_address_binding(
                &lmdata->application_data[pos], lmdata->application_max_data_len - pos, &object_id, &route.bacnetPath);
            address_add(object_id.instance, MAX_APDU/*max_apdu*/, &route);
            pos += len;
        }
        lmdata->application_data_len = pos;
        return true;
    case PROP_VT_CLASSES_SUPPORTED:
    case PROP_ACTIVE_VT_SESSIONS:
    case PROP_LIST_OF_SESSION_KEYS:
    case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
    case PROP_ACTIVE_COV_SUBSCRIPTIONS:
    case PROP_SLAVE_ADDRESS_BINDING:
        lmdata->first_failed_element = 1;
        lmdata->error_class = ERROR_CLASS_PROPERTY;
        lmdata->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    default:
        lmdata->error_class = ERROR_CLASS_SERVICES;
        lmdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
        return false;
    }
}

#if 0
bool Device_Remove_List_Element_Local(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_LIST_MANIPULATION_DATA* lmdata) {
    BACNET_OBJECT_ID object_id;
    BACNET_PATH address;
    //    BACNET_PATH binding;
    //    unsigned max_apdu;
    int end = lmdata->application_max_data_len;
    int pos = 0;
    int len = 0;

    switch (lmdata->object_property) {

    case PROP_DEVICE_ADDRESS_BINDING:
        /* validate list elements */
        while (pos < end) {
            len = Device_decode_address_binding(
                &lmdata->application_data[pos], lmdata->application_max_data_len - pos, &object_id, &address);
            /* decode error */
            if (len < 0) {
                lmdata->error_class = ERROR_CLASS_PROPERTY;
                lmdata->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                return false;
            }
            /* matching address entry not in list? */
            if (!address_get_by_device(object_id.instance, &max_apdu, &binding) || !address_match(&address, &binding)) {
#if (BACNET_PROTOCOL_REVISION >= 7)
                lmdata->error_class = ERROR_CLASS_PROPERTY;
                lmdata->error_code = ERROR_CODE_INVALID_DATA_TYPE;
#else
                lmdata->error_class = ERROR_CLASS_SERVICES;
                lmdata->error_code = ERROR_CODE_OTHER;
#endif
                return false;
            }
            lmdata->first_failed_element++;
            pos += len;
        }
        pos = 0;
        /* remove the list elements from the address cache */
        while (pos < end) {
            len = Device_decode_address_binding(
                &lmdata->application_data[pos], lmdata->application_max_data_len - pos, &object_id, &address);
            address_remove_device(object_id.instance);
            pos += len;
        }
        lmdata->application_data_len = pos;
        return true;

    case PROP_VT_CLASSES_SUPPORTED:
    case PROP_ACTIVE_VT_SESSIONS:
    case PROP_LIST_OF_SESSION_KEYS:
    case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
    case PROP_ACTIVE_COV_SUBSCRIPTIONS:
    case PROP_SLAVE_ADDRESS_BINDING:
        lmdata->first_failed_element = 1;
        lmdata->error_class = ERROR_CLASS_PROPERTY;
        lmdata->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    default:
        lmdata->error_class = ERROR_CLASS_SERVICES;
        lmdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
        return false;
    }
}
#endif
#endif

/** Initialize the Device Function Tables.
 Initialize the group of object helper functions for any supported Object.
 Initialize each of the Device Object child Object instances.
 * @ingroup ObjIntf
 * @param object_table [in,out] array of structure with object functions.
 *  Each Child Object must provide some implementation of each of these
 *  functions in order to properly support the default handlers.
 */
void Device_Object_Function_Tables_Init(
    void) {
    struct object_functions* pObject = &My_Object_Table[0];

    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Init) {
            pObject->Object_Init();
        }
        pObject++;
    }

    Device_Time_Init();
}


extern PORT_SUPPORT* applicationDatalink;       // Where we put the router Application Entity device

static void Device_Init_Common(
    PORT_SUPPORT* datalink,
    DEVICE_OBJECT_DATA* pDev,
    uint32_t deviceInstance
) {
    pDev->datalink = datalink;

    pDev->bacObj.objectInstance = deviceInstance;
    pDev->bacObj.objectType = OBJECT_DEVICE;

    ll_Init(&pDev->AI_Descriptor_List, 1000);
    ll_Init(&pDev->AO_Descriptor_List, 1000);
    ll_Init(&pDev->AV_Descriptor_List, 1000);
    ll_Init(&pDev->BI_Descriptor_List, 1000);
    ll_Init(&pDev->BO_Descriptor_List, 1000);
    ll_Init(&pDev->BV_Descriptor_List, 1000);
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
    ll_Init(&pDev->MSI_Descriptor_List, 1000);
    ll_Init(&pDev->MSO_Descriptor_List, 1000);
    ll_Init(&pDev->MSV_Descriptor_List, 1000);
#endif
#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
    ll_Init(&pDev->NC_Descriptor_List, 1000);
#endif
    ll_Init(&pDev->Calendar_Descriptor_List, 1000);
    ll_Init(&pDev->Schedule_Descriptor_List, 1000);
    ll_Init(&pDev->TrendLog_Descriptor_List, 1000);
#if (BACNET_PROTOCOL_REVISION >= 17)
    ll_Init(&pDev->NP_Descriptor_List, 1000);
#endif

}


void Init_ServerSide(void) {

#if ( VIRTUALDEV == 1 )
    // The +1 is to accommodate the router Device Entity
    // we really should think about putting this list of devices on the appropriate datalink MX_VIRTUAL_DEVICES on the virtual routerport's datalink
    // and 1 on the application datalink for the router's Application Entity
    // but what about a single device (client or server) supporting multiple datalinks? (non-routing version) or should we ban that?
    ll_Init(&serverDeviceCB, MX_VIRTUAL_DEVICES+1);
#else
    ll_Init(&serverDeviceCB, 1 );
#endif // #if ( VIRTUALDEV == 1 )

}


DEVICE_OBJECT_DATA* Device_Create_Device_Server(
    PORT_SUPPORT* datalink,
    uint32_t deviceInstance,
    const char* deviceName,
    const char* deviceDescription,
    const char* modelName,
    const void* userDeviceData) {

    // make sure we are not trying to create a duplicate
    if (Device_Find_Device(deviceInstance)) {
        dbMessage(DBD_Config, DB_ERROR, "Server Device Instance [%u] already exists. We will not create a duplicate now.", deviceInstance);
        return NULL;
    }

    DEVICE_OBJECT_DATA* pDev = (DEVICE_OBJECT_DATA*)emm_scalloc('d', (uint16_t) sizeof(DEVICE_OBJECT_DATA));
    if (!emm_check_alloc(pDev)) {
        panic();
        return NULL;
    }

    Device_Init_Common(datalink, pDev, deviceInstance);

    characterstring_init_ansi(&pDev->bacObj.objectName, deviceName);
    characterstring_init_ansi(&pDev->bacObj.description, deviceDescription);
    characterstring_init_ansi(&pDev->Model_Name, modelName);
    characterstring_init_ansi(&SerialNumber, "Default 02");

#if ( BACNET_SVC_COV_B == 1 )
    handler_cov_init(pDev);
#endif

    if (!ll_Enqueue(&serverDeviceCB, pDev)) {
        panic();
        return NULL ;
    }

    return pDev;
}


void Device_Remove_Objects(LLIST_HDR *objListHdr)
{
    while (objListHdr->count)
    {
        void *currentObject = ll_Dequeue(objListHdr);
        emm_free(currentObject);
    }
}


void Device_Delete_Device(DEVICE_OBJECT_DATA* pDev, const char* protocol)
{
    if (Device_Find_Device(pDev->bacObj.objectInstance))
    {
        ll_Remove(&serverDeviceCB, pDev);
    }
    else
    {
        panic();
        return;
    }

#if ( BACNET_SVC_COV_B == 1 )
    handler_cov_deinit(pDev);
#endif

    // remove all objects that may have been created and attached to this device
    Device_Remove_Objects(&pDev->AI_Descriptor_List);
    Device_Remove_Objects(&pDev->AO_Descriptor_List);
    Device_Remove_Objects(&pDev->AV_Descriptor_List);
    Device_Remove_Objects(&pDev->BI_Descriptor_List);
    Device_Remove_Objects(&pDev->BO_Descriptor_List);
    Device_Remove_Objects(&pDev->BV_Descriptor_List);
#if (BACNET_USE_OBJECT_CHARACTER_STRING_VALUE == 1 )
    Device_Remove_Objects(&pDev->CSV_Descriptor_List);
#endif
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
    Device_Remove_Objects(&pDev->MSI_Descriptor_List);
    Device_Remove_Objects(&pDev->MSO_Descriptor_List);
    Device_Remove_Objects(&pDev->MSV_Descriptor_List);
#endif
    Device_Remove_Objects(&pDev->NC_Descriptor_List);
    Device_Remove_Objects(&pDev->NP_Descriptor_List);
    Device_Remove_Objects(&pDev->Calendar_Descriptor_List);
    Device_Remove_Objects(&pDev->Schedule_Descriptor_List);
    Device_Remove_Objects(&pDev->TrendLog_Descriptor_List);

    if (pDev->bacObj.userData) {
        emm_free(pDev->bacObj.userData);
    }
 
    emm_free(pDev);
}


void AnnounceDevices(void) {
}

#if  (BACNET_SVC_RR_B == 1)

bool DeviceGetRRInfo(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_RANGE_DATA* pRequest,  /* Info on the request */
    RR_PROP_INFO* pInfo) {       /* Where to put the response */
    bool status = false;        /* return value */

    switch (pRequest->object_property) {
    case PROP_VT_CLASSES_SUPPORTED:
    case PROP_ACTIVE_VT_SESSIONS:
    case PROP_LIST_OF_SESSION_KEYS:
    case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
    case PROP_MANUAL_SLAVE_ADDRESS_BINDING:
    case PROP_SLAVE_ADDRESS_BINDING:
    case PROP_RESTART_NOTIFICATION_RECIPIENTS:
    case PROP_UTC_TIME_SYNCHRONIZATION_RECIPIENTS:
    case PROP_ACTIVE_COV_MULTIPLE_SUBSCRIPTIONS:
        pInfo->RequestTypes = RR_BY_POSITION;
        pRequest->error_class = ERROR_CLASS_PROPERTY;
        pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;

    case PROP_DEVICE_ADDRESS_BINDING:
        // this is a required property, therefore the RR service has to exist, but it will always encode nothing..
        // if there is no need for an address list due to #if  ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1)
        pInfo->RequestTypes = RR_BY_POSITION;
        pInfo->Handler = rr_address_list_encode;
        status = true;
        break;

    case PROP_ACTIVE_COV_SUBSCRIPTIONS:
        pInfo->RequestTypes = RR_BY_POSITION;
#if ( BACNET_SVC_COV_B == 1 )
        pRequest->error_class = ERROR_CLASS_PROPERTY;
        pRequest->error_code = ERROR_CODE_READ_ACCESS_DENIED;
        status = false;
#else
        pRequest->error_class = ERROR_CLASS_PROPERTY;
        pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
#endif
        break;

    default:
        pRequest->error_class = ERROR_CLASS_SERVICES;
        pRequest->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
        break;
    }

    return status;
}
#endif


DEVICE_OBJECT_DATA* Device_Find_Device(
    const uint32_t deviceInstance) {
    DEVICE_OBJECT_DATA* pDev = (DEVICE_OBJECT_DATA*)ll_First(&serverDeviceCB);
    while (pDev != NULL) {
        if (pDev->bacObj.objectInstance == deviceInstance) return pDev;
        pDev = (DEVICE_OBJECT_DATA*)ll_Next(&serverDeviceCB, (void*)pDev);
    }
    return NULL;
}






#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE* pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS* pErrorClass,
    BACNET_ERROR_CODE* pErrorCode) {
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

bool WPValidateString(
    BACNET_APPLICATION_DATA_VALUE* pValue,
    uint16_t iMaxLen,
    bool bEmptyAllowed,
    BACNET_ERROR_CLASS* pErrorClass,
    BACNET_ERROR_CODE* pErrorCode) {
    pValue = pValue;
    iMaxLen = iMaxLen;
    bEmptyAllowed = bEmptyAllowed;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

int handler_cov_encode_subscriptions(
    uint8_t* apdu,
    int max_apdu) {
    apdu = apdu;
    max_apdu = max_apdu;

    return 0;
}

void testDevice(
    Test* pTest) {
    bool status;
    const char* name = "Patricia";

    status = Device_Set_Object_Instance_Number(0);
    ct_test(pTest, Device_Object_Instance_Number() == 0);
    ct_test(pTest, status == true);
    status = Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    ct_test(pTest, Device_Object_Instance_Number() == BACNET_MAX_INSTANCE);
    ct_test(pTest, status == true);
    status = Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE / 2);
    ct_test(pTest,
        Device_Object_Instance_Number() == (BACNET_MAX_INSTANCE / 2));
    ct_test(pTest, status == true);
    status = Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE + 1);
    ct_test(pTest,
        Device_Object_Instance_Number() != (BACNET_MAX_INSTANCE + 1));
    ct_test(pTest, status == false);


    Device_Set_System_Status(STATUS_NON_OPERATIONAL, true);
    ct_test(pTest, Device_System_Status() == STATUS_NON_OPERATIONAL);

    ct_test(pTest, Device_Vendor_Identifier() == BACNET_VENDOR_ID);

    Device_Set_Model_Name(name, strlen(name));
    ct_test(pTest, strcmp(Device_Model_Name(), name) == 0);

}

#ifdef TEST_DEVICE
int main(
    void) {
    Test* pTest;
    bool rc;

    pTest = ct_create("BACnet Device", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testDevice);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_DEVICE */
#endif /* TEST */
