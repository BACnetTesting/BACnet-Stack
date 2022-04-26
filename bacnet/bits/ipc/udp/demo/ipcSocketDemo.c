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

#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "..\ipcApplicationSide.h"

#define BACNET_TEST_DEVICE_ID   3046210

void ipc_callBackAnaUpdate(uint deviceID, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propId, float val)
{
    // this function is called by the BACnet stack when any property is updated with a BACnet Real value
}

void ipc_callBackBinUpdate(uint deviceID, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propId, bool val)
{
    // this function is called by the BACnet stack when any property is updated with a BACnet Boolean value
}

void ipc_callBackEnumUpdate(uint deviceID, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propId, uint val)
{
    // this function is called by the BACnet stack when any property is updated with a BACnet Boolean value
}

void ipc_callBackStringUpdate(uint deviceID, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propId, char *val)
{
    // this function is called by the BACnet stack when any property is updated with a BACnet Character String value
}

void ipc_callBackDeviceStatusUpdate(uint deviceID, bool status)
{
}

extern bool showIPChandshake;
int testMode;
bool online = true ;


void SetupTestConfiguration(void)
{
    // todo - check if there are multiple adapters / interfaces and issue a suitable warning...

    uint portIdIP   = 1;                              // links Device to Network
    uint portIdVirt = 2;
    uint portIdMSTP = 3;

    uint deviceRouterInst = 10000;
    uint deviceVirtInst1 = BACNET_TEST_DEVICE_ID;         // links Object to Device
    uint deviceVirtInst2 = BACNET_TEST_DEVICE_ID+1;

    // Set up the networks...
#ifdef _MSC_VER
    // BACnet port set to 60701 for BBMD testing on same (Windows) machine
    ipcClient_create_network_IP(portIdIP, 1111, "Any", 60701, true);
#else
    // BACnet port set to default 47808 because BACnet running on target/VM
    ipcClient_create_network_IP(portIdIP, 1111, "eth0", 47808, true);
#endif

    ipcClient_create_network_virtual(portIdVirt, 2222 );
    // future ipcClient_create_MSTP_network(portId, 0, "Any", 60701, true);

    // Set up the "Application Entity" for the Virtual Router, specifying where we want it homed.
    ipcClient_create_device_router(portIdIP, deviceRouterInst, "Virtual Router", "This is the 'Virtual Router' 'Application Entity', homed on the IP Network");

    // Set up all the Virtual devices, and their BACnet Objects, these are to be mapped onto the Modbus devices and Modbus Registers, Coils.

    ipcClient_create_device_virtual(portIdVirt, deviceVirtInst1, "MASC Virtual Device 1", "This is the first MASC device");

    ipcClient_create_object(deviceVirtInst1, OBJ_TYPE_AI, 100, "AI_100", "First Dummy Analog Object's Description");
    ipcClient_create_object(deviceVirtInst1, OBJ_TYPE_AI, 101, "AI_101", "Second Dummy Analog Object's Description");
    ipcClient_create_object(deviceVirtInst1, OBJ_TYPE_AI, 102, "AI_102", "Third Dummy Analog Object's Description");

    ipcClient_create_device_virtual(portIdVirt, deviceVirtInst2, "Virtual Device 2", "This is the second device");

    ipcClient_create_object(deviceVirtInst2, OBJ_TYPE_AI, 100, "AI_100", "First Dummy Analog Object's Description");
    ipcClient_create_object(deviceVirtInst2, OBJ_TYPE_AI, 101, "AI_101", "Second Dummy Analog Object's Description");
    ipcClient_create_object(deviceVirtInst2, OBJ_TYPE_AI, 102, "AI_102", "Third Dummy Analog Object's Description");


    ipcClient_create_device_virtual(portIdVirt, deviceVirtInst3, "Virtual Device 3", "This is the third device");

    ipcClient_create_object(deviceVirtInst3, OBJ_TYPE_AI, 1, "AI_1", "Analog Input Object's Description");
    ipcClient_create_object(deviceVirtInst3, OBJ_TYPE_AO, 1, "AO_1", "Analog Output Object's Description");
    ipcClient_create_object(deviceVirtInst3, OBJ_TYPE_AV, 1, "AV_1", "Analog Value Object's Description");
    ipcClient_create_object(deviceVirtInst3, OBJ_TYPE_BI, 1, "BI_1", "Binary Input Object's Description");
    ipcClient_create_object(deviceVirtInst3, OBJ_TYPE_BO, 1, "BO_1", "Binary Output Object's Description");
    ipcClient_create_object(deviceVirtInst3, OBJ_TYPE_BV, 1, "BV_1", "Binary Value Object's Description");

}


void DoHelp(void)
{
    printf("\nType:  Q - quit");
    printf("\n       R - Restart");
    printf("\n       M - Set M4 Mode (MSTP/ModbusRTU)");
    printf("\n       B - Set M4 RS485 BAUD rate (MSTP/ModbusRTU)");
    printf("\n       C - Create Device");
    printf("\n       D - Create Object");
    printf("\n       V - Update Object PV");
    printf("\n       P - Toggle PingPong view (comms activity) (currently %d)", showIPChandshake);
    printf("\n       O - Toggle Device %d Online/Offline (currently %s)", BACNET_TEST_DEVICE_ID, (online) ? "Online":"Offline" );
    printf("\n       M - Change TestMode (Ana/Bin) (currently %d)", testMode );
    printf("\n       T - Setup Test Configuration");
    printf("\n");
}

bool ProcessKey(int ch)
{
    switch (toupper(ch) )
    {
    case 'Q': 
        return false;
    case 'R':
        ipcClient_send_restart();
        printf("\nRestart request sent\n");
        break;
    case 'M':
        testMode++;
        if (testMode > 1) testMode = 0;
        DoHelp();
        break;
    case 'P':
        showIPChandshake = !showIPChandshake;
        DoHelp();
        break;
    case 'O':
        online = !online;
        ipcClient_update_device_status(BACNET_TEST_DEVICE_ID, online);
        break;
    case 'T':
        SetupTestConfiguration();
        printf("\nTest Configuration set up\n");
        break;
    default:
        DoHelp();
        break;
    }
    return true;
}


int main()
{
    time_t oldTime, timeNow ;
    time(&oldTime);
    float value = 17.0f;
    int bvalue = 1;

    // fire up the listener thread
    bool status = ipcClient_init_Application();
    if (!status)
    {
        panic();
        return -1;
    }

    DoHelp();

    bool busy = true;
    while (busy)
    {
        if (bitsKBhit()) busy = ProcessKey(bitsGetch());

        // do some update mischief - every 10 seconds
        time(&timeNow);
        if ( timeNow - oldTime > 10 )
        { 
            oldTime = timeNow;

            switch (testMode)
            {
            case 0:
                // Analog Test Mode 
                printf("\nSending AI update, new value %f", value);
                ipcClient_update_object_analog(BACNET_TEST_DEVICE_ID, OBJ_TYPE_AI, 100, PROP_PV, value);
                value += 0.74f;
                break;

            case 1:
                // Binary Test mode
                printf("\nSending BI update, new value %d", bvalue);
                ipcClient_update_object_binary(BACNET_TEST_DEVICE_ID, OBJ_TYPE_BI, 100, PROP_PV, bvalue);
                bvalue = !bvalue;
                break;
            }
        }
       
        msSleep(100);
    }
}
