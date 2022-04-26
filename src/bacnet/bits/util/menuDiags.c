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

#ifdef _MSC_VER
#include <conio.h>
#endif

#include "osLayer.h"
#include "eLib/util/logging.h"
#include "bitsUtil.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "appApi.h"
// #include "objectModel.h"
#include "bacnet/bacenum.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/basic/object/ao.h"

#if ( BACNET_CLIENT == 1 )
void print_address_cache(PORT_SUPPORT* datalink);
#endif

#if ( IPC )
#include "ipcBACnetSide.h"      // for CreateObject, for now
#endif

#include "multipleDatalink.h"
#include "eLib/util/eLibDebug.h"
#include "menuDiags.h"

#include "bacnet/basic/object/ai.h"

extern PORT_SUPPORT* datalinkSupportHead;
extern ROUTER_PORT* applicationRouterPort;

#if ( BITS_USE_IPC == 1 )
extern bool showIPChandshake;
#endif



#if 0
#if defined(BBMD_ENABLED) && BBMD_ENABLED
static void ShowBDT(void)
{
    bool entries = false;
    log_printf("BBMD Device Table:");
    for (int i = 0; i < MAX_BBMD_ENTRIES; i++) {
        BBMD_TABLE_ENTRY* bte = &BBMD_Table[i];
        if (bte->valid) {
            entries = true;
            char tstring[100];
            log_printf("   %s", IPAddr_Port_ToString(tstring, &bte->dest_address, bte->dest_port));
        }
    }
    if (!entries) log_printf("   No entries.");
}

static void ShowFDT(void)
{
    bool entries = false;

    // show active FDRs
    log_printf("Foreign Device Registrations:");
    if (Remote_BBMD.sin_addr.s_addr) {
        char tstring[100];
        log_printf("   %s", IPEP_ToString(tstring, &Remote_BBMD));
    }
    else {
        log_printf("   No entries.");
    }

    log_printf("Foreign Device Table:");
    for (int i = 0; i < MAX_FD_ENTRIES; i++) {
        FD_TABLE_ENTRY* bte = &FD_Table[i];
        if (bte->dest_address.s_addr) {
            entries = true;
            char tstring[100];
            log_printf("   %s,  TTL:%d", IPAddr_Port_ToString(tstring, &bte->dest_address, bte->dest_port), bte->time_to_live);
        }
    }
    if (!entries) log_printf("   No entries.");
}
#endif
#endif


static ROUTER_PORT* thisRouterport = NULL;


static void ShowDevice(VirtualDeviceInfo* vDev)
{
    log_printf("   Device: ID:%6d, Mac:%02x  AI:%d AO:%d AV:%d  BI:%d BO:%d BV:%d  MSI:%d MSO:%d MSV:%d  NC:%d Port:%d",
        vDev->pDev->bacObj.objectInstance,
        vDev->virtualMACaddr.bytes[0],
        vDev->pDev->AI_Descriptor_List.count,
        vDev->pDev->AO_Descriptor_List.count,
        vDev->pDev->AV_Descriptor_List.count,
        vDev->pDev->BI_Descriptor_List.count,
        vDev->pDev->BO_Descriptor_List.count,
        vDev->pDev->BV_Descriptor_List.count,
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
        vDev->pDev->MSI_Descriptor_List.count,
        vDev->pDev->MSO_Descriptor_List.count,
        vDev->pDev->MSV_Descriptor_List.count,
#else
        0,0,0,
#endif
#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
		vDev->pDev->NC_Descriptor_List.count,
#else
        0,
#endif
#if (BACNET_PROTOCOL_REVISION >= 17)
		vDev->pDev->NC_Descriptor_List.count
#else
		0
#endif
    );


    log_printf("                               Cal:%d  Sched:%d  TrendLog:%d",
        vDev->pDev->Calendar_Descriptor_List.count,
        vDev->pDev->Schedule_Descriptor_List.count,
        vDev->pDev->TrendLog_Descriptor_List.count
    );

#if 0
    vDev->pDev->networkPorts.size(),
        vDev->pDev->notificationClasses.size());
#endif
}


extern LLIST_HDR serverDeviceCB;
extern DEVICE_OBJECT_DATA* displayDev;

static void ShowNextObject(void)
{
    if (displayDev == NULL) return;

    BACNET_OBJECT* currentObject;

#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
    // see cr32471041024124 
    // this needs a bit of thought...
    dbMessage(DBD_UI, DB_ALWAYS, "Analog Inputs");
    currentObject = (BACNET_OBJECT*)ll_First(&displayDev->AI_Descriptor_List);
    while (currentObject) {
        ANALOG_INPUT_DESCR* curObject = (ANALOG_INPUT_DESCR*)currentObject;
        dbMessage(DBD_UI, DB_ALWAYS, "   AI:%u  = %f", curObject->analogCommon.common.objectInstance, curObject->analogCommon.Present_Value);
        currentObject = (BACNET_OBJECT*)ll_Next(&displayDev->AI_Descriptor_List, currentObject);
    }

    dbMessage(DBD_UI, DB_ALWAYS, "Analog Outputs");
    currentObject = (BACNET_OBJECT*)ll_First(&displayDev->AO_Descriptor_List);
    while (currentObject)
    {
        ANALOG_OUTPUT_DESCR* curObject = (ANALOG_OUTPUT_DESCR*)currentObject;
        dbMessage(DBD_UI, DB_ALWAYS, "   AO:%u  = %f", curObject->analogCommon.common.objectInstance, curObject->analogCommon.Present_Value);
        currentObject = (BACNET_OBJECT*)ll_Next(&displayDev->AO_Descriptor_List, currentObject);
    }
#endif

#if 0
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->analogInputs.begin(); iObject != displayDev->pDev->analogInputs.end(); ++iObject)
    {
        log_printf("AI:%u  = %f", (*iObject)->Object_Instance_Number, ((BACnetAnalogObject*)(*iObject))->Present_Value);
    }
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->analogOutputs.begin(); iObject != displayDev->pDev->analogOutputs.end(); ++iObject)
    {
        log_printf("AO:%u  = %f", (*iObject)->Object_Instance_Number, ((BACnetAnalogObject*)(*iObject))->Present_Value);
    }
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->analogValues.begin(); iObject != displayDev->pDev->analogValues.end(); ++iObject)
    {
        log_printf("AV:%u  = %f", (*iObject)->Object_Instance_Number, ((BACnetAnalogObject*)(*iObject))->Present_Value);
    }

    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->binaryInputs.begin(); iObject != displayDev->pDev->binaryInputs.end(); ++iObject)
    {
        log_printf("BI:%u  = %u", (*iObject)->Object_Instance_Number, ((BACnetBinaryObject*)(*iObject))->Present_Value);
    }
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->binaryOutputs.begin(); iObject != displayDev->pDev->binaryOutputs.end(); ++iObject)
    {
        log_printf("BO:%u  = %u", (*iObject)->Object_Instance_Number, ((BACnetBinaryObject*)(*iObject))->Present_Value);
    }
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->binaryValues.begin(); iObject != displayDev->pDev->binaryValues.end(); ++iObject)
    {
        log_printf("BV:%u  = %u", (*iObject)->Object_Instance_Number, ((BACnetBinaryObject*)(*iObject))->Present_Value);
    }

    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->multistateInputs.begin(); iObject != displayDev->pDev->multistateInputs.end(); ++iObject)
    {
        log_printf("MSI:%u = %u", (*iObject)->Object_Instance_Number, ((BACnetMultistateObject*)(*iObject))->Present_Value);
    }
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->multistateOutputs.begin(); iObject != displayDev->pDev->multistateOutputs.end(); ++iObject)
    {
        log_printf("MSO:%u = %u", (*iObject)->Object_Instance_Number, ((BACnetMultistateObject*)(*iObject))->Present_Value);
    }
    for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->multistateValues.begin(); iObject != displayDev->pDev->multistateValues.end(); ++iObject)
    {
        log_printf("MSV:%u = %u", (*iObject)->Object_Instance_Number, ((BACnetMultistateObject*)(*iObject))->Present_Value);
    }



}
for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->analogOutputs.begin(); iObject != displayDev->pDev->analogOutputs.end(); ++iObject)
{
    log_printf("AO:%u  = %f", (*iObject)->Object_Instance_Number, ((BACnetAnalogObject*)(*iObject))->Present_Value);
}
for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->analogValues.begin(); iObject != displayDev->pDev->analogValues.end(); ++iObject)
{
    log_printf("AV:%u  = %f", (*iObject)->Object_Instance_Number, ((BACnetAnalogObject*)(*iObject))->Present_Value);
}

for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->binaryInputs.begin(); iObject != displayDev->pDev->binaryInputs.end(); ++iObject)
{
    log_printf("BI:%u  = %u", (*iObject)->Object_Instance_Number, ((BACnetBinaryObject*)(*iObject))->Present_Value);
}
for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->binaryOutputs.begin(); iObject != displayDev->pDev->binaryOutputs.end(); ++iObject)
{
    log_printf("BO:%u  = %u", (*iObject)->Object_Instance_Number, ((BACnetBinaryObject*)(*iObject))->Present_Value);
}
for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->binaryValues.begin(); iObject != displayDev->pDev->binaryValues.end(); ++iObject)
{
    log_printf("BV:%u  = %u", (*iObject)->Object_Instance_Number, ((BACnetBinaryObject*)(*iObject))->Present_Value);
}

for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->multistateInputs.begin(); iObject != displayDev->pDev->multistateInputs.end(); ++iObject)
{
    log_printf("MSI:%u = %u", (*iObject)->Object_Instance_Number, ((BACnetMultistateObject*)(*iObject))->Present_Value);
}
for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->multistateOutputs.begin(); iObject != displayDev->pDev->multistateOutputs.end(); ++iObject)
{
    log_printf("MSO:%u = %u", (*iObject)->Object_Instance_Number, ((BACnetMultistateObject*)(*iObject))->Present_Value);
}
for (std::vector<BACnetObject*>::iterator iObject = displayDev->pDev->multistateValues.begin(); iObject != displayDev->pDev->multistateValues.end(); ++iObject)
{
    log_printf("MSV:%u = %u", (*iObject)->Object_Instance_Number, ((BACnetMultistateObject*)(*iObject))->Present_Value);
}
#endif    


}


void bits_showMenuHelp(void)
{
    dbMessage(DBD_UI, DB_ALWAYS, "");
    dbMessage(DBD_UI, DB_ALWAYS, "Keys: Q) uit");
    dbMessage(DBD_UI, DB_ALWAYS, "      L) Datalinks          D) Next Datalink");
#if (BITS_ROUTER_LAYER == 1)
    dbMessage(DBD_UI, DB_ALWAYS, "      R) RouterPorts        P) Next RouterPort");
#endif

#if (BACNET_CLIENT == 1)
    dbMessage(DBD_UI, DB_ALWAYS, "      D) Next Srv/V/A Dev   M) Next Client-side Device");
    dbMessage(DBD_UI, DB_ALWAYS, "      O) Next Object        G) Next Poll Record");
    dbMessage(DBD_UI, DB_ALWAYS, "      A) App Binding Table");
#endif

    // log_printf("      E) EMM trace log    +/-) Incr/Decr debug (%u)", dbMessage_Level_Get() );

#if ( BITS_USE_IPC == 1 )
    dbMessage(DBD_UI, DB_ALWAYS, "      I) Show IPC handshake (%s)", showIPChandshake ? "On" : "Off");
#endif
}


LLIST_HDR menuStack;

// returning true means we want to continue operation
bool bits_diagnosticMenu(char key)
{

    if (menuStack.max == 0) ll_Init(&menuStack,10);
    if (menuStack.count)
    {
        bool weProcessedThisKey = true;
        ACTIVE_MENU* menu = (ACTIVE_MENU *) ll_First( &menuStack);
        if (key == menu->keyOnEnter) {
            if (menu->NextItem) menu->NextItem();
            weProcessedThisKey = true ;
        }
        else {
            switch (key) {
            case 'J':
                menu->NextItem();
                break;
            case 'L':
            case '\n':
                menu->EnterItem();
                break;
            case 'H':
                menu->HelpScreen();
                break;
            case 27 :   // esc
                menu->ExitMenu();
                ll_Remove(&menuStack, menu);
                break;
            default:
                weProcessedThisKey = menu->HandleKey(key) ;
                break;
            }
        }

        if (weProcessedThisKey) return true ;
        // else continue with other keys since we 
    }

    switch (key) {

    case '+':
        dbMessage_Level_Incr();
        break;

    case '-':
        dbMessage_Level_Decr();
        break;

#if ( BACNET_CLIENT == 1 )
    case 'A':
        print_address_cache( applicationRouterPort->port_support );
        break;
#endif

#if defined(BBMD_ENABLED) && BBMD_ENABLED
    case 'B':
        // ShowBDT();
        break;

    case 'F':
        // ShowFDT();
        break;
#endif

#if ( BACNET_CLIENT == 1 )
    case 'G':
        dm_PollRecord_Setup(key);
        break;
#endif

#if ( BITS_USE_IPC == 1 )
    case 'I':
        showIPChandshake = !showIPChandshake;
        ShowMenuHelp();
        break;
#endif

    case 'v':
        dm_ServerDevice_Setup( key );
        break;

#if ( BACNET_CLIENT == 1 )
    case 'M':
        dm_ClientDevice_Setup(key);
        break;
#endif

    case 'L':
        dm_Datalink_Setup(key);
        break;

    case 'O':
        ShowNextObject();
        break;

#if (BITS_ROUTER_LAYER == 1)
    case 'P':
        dm_RouterPort_Setup(key);
        break;
#endif

    case 'Q':
        return false;

#if (BITS_ROUTER_LAYER == 1)
    case 'R':
        ShowRouterPorts();
#endif
    }

    return true;
}




