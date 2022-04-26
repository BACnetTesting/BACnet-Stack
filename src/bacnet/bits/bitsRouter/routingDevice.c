/****************************************************************************************
 *
 *   Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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

#include "configProj.h"

#if ( BITS_ROUTER_LAYER == 1 )

#include <stdbool.h>

#include <stdint.h>
#include <string.h>     /* for memmove */
#include <time.h>       /* for timezone, localtime */
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
#include "bacnet/bits/util/multipleDatalink.h"
#include "bacnet/basic/binding/address.h"
#include "eLib/util/emm.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"

extern ROUTER_PORT* headRouterPort;

uint32_t virtualRouterDeviceInstance;

VirtualDeviceInfo* Device_Find_VirtualDevice(
    const uint32_t deviceInstance)
{
    ROUTER_PORT* frp = headRouterPort;
    while (frp != NULL) {
        if (frp->port_support->portType == BPT_VIRT) {

            VirtualDeviceInfo* virtualDevice = (VirtualDeviceInfo*)ll_First(&frp->datalinkDevicesVirtual);
            while (virtualDevice != NULL) {
                if (virtualDevice->pDev->bacObj.objectInstance == deviceInstance) return virtualDevice;
                virtualDevice = (VirtualDeviceInfo*)ll_Next(&frp->datalinkDevicesVirtual, virtualDevice);
            }
        }
        frp = (ROUTER_PORT*)frp->llist.next;
    }
    return NULL;
}

void Device_Remove_VirtualDevice(
    const uint32_t deviceInstance)
{
    ROUTER_PORT* frp = headRouterPort;
    while (frp != NULL) {
        if (frp->port_support->portType == BPT_VIRT) {
            VirtualDeviceInfo* virtualDevice = (VirtualDeviceInfo*)ll_First(&frp->datalinkDevicesVirtual);
            while (virtualDevice != NULL) {
                if (virtualDevice->pDev->bacObj.objectInstance == deviceInstance) {
                    ll_Remove(&frp->datalinkDevicesVirtual, virtualDevice);
                    emm_free(virtualDevice);
                    return;
                }
                virtualDevice = (VirtualDeviceInfo*)ll_Next(&frp->datalinkDevicesVirtual, virtualDevice);
            }
        }
        frp = (ROUTER_PORT*)frp->llist.next;
    }
    dbMessage(DBD_Config, DB_NOTE, "Device_Remove_VirtualDevice : Device Instance [%u] Not found", deviceInstance);
}

extern ROUTER_PORT* associatedRouterPort;
extern ROUTER_PORT* applicationRouterPort;

void Create_Device_Router(const uint8_t portId, const unsigned devInstance, const char* devName, const char* devDesc, const unsigned vendorId, const char* vendorName)
{
    // Find the Datalink we are going to attach our device to. The user is responsible for creating a routerport to use
    // we will backfill associated routerport at the right (and only) time

    ROUTER_PORT* frp = headRouterPort;
    PORT_SUPPORT* datalink = NULL;
    while (frp != NULL) {
        if (frp->port_id == portId) {
            datalink = frp->port_support;
            break;
        }
        frp = (ROUTER_PORT*)frp->llist.next;
    }

    if (datalink == NULL) {
        panicDesc("Create datalink prior to attempting to create Router Device");
        return;
    }

    // this should be the one and only time!
    if (applicationRouterPort != NULL) {
        dbMessage(DBD_Config, DB_NOTE, "ApplicationRouterPort already exists. Ignoring subsequent attempt.");
        return;
    }

    // time to create on on behalf of the user. Using same network number
    bool ok = InitRouterportApp(frp->route_info.configuredNet.net);          // Network Number to 'associate' with.
    if (!ok) {
        panic();
        return;
    }

    // and associate app with prior frp
    AlignApplicationWithPort();

    //    DEVICE_OBJECT_DATA *device = new DEVICE_OBJECT_DATA();
    //    if (device == NULL) {
    //        panic();
    //        return;
    //    }

    virtualRouterDeviceInstance = devInstance;

    Virtual_Router_Init(devInstance, devName, devDesc);
}



VirtualDeviceInfo* Create_Device_Virtual_VMAC (
    const uint8_t portId, 
    const unsigned devInstance, 
    const char* devName, 
    const char* devDesc, 
    const unsigned vendorId, 
    const char* vendorName,
    const uint16_t vMAClocal ) {
    // Find the Datalink we are going to attach our device to. The user is resposible for creating a routerport to use
    // we will backfill associated routerport at the right (and only) time

    ROUTER_PORT* frp = headRouterPort;
    PORT_SUPPORT* datalink = NULL;
    while (frp != NULL) {
        if (frp->port_id == portId) {
            datalink = frp->port_support;
            break;
        }
        frp = (ROUTER_PORT*)frp->llist.next;
    }

    if (datalink == NULL) {
        panicDesc("User did not create datalink prior to attempting to create Device");
        return NULL;
    }

    if (frp->port_support->portType != BPT_VIRT) {
        panicDesc("We expect only virtual devices to be allocated to virtual networks");
        return NULL;
    }

    // make sure we are not trying to create a duplicate
    if (Device_Find_VirtualDevice(devInstance))
    {
        dbMessage(DBD_Config, DB_ERROR, "Device Instance [%u] already exists. We will not create a duplicate now. 3", devInstance);
        return NULL;
    }

    DEVICE_OBJECT_DATA* pDev = Device_Create_Device_Server(datalink, devInstance, devName, devDesc, "VirtDev", NULL);

    VirtualDeviceInfo* vDev = (VirtualDeviceInfo*)emm_scalloc('e', (uint16_t) sizeof(VirtualDeviceInfo));
    if (!emm_check_alloc_two(vDev, pDev)) {
        panic();
        return NULL;
    }

    vDev->pDev = pDev;
    pDev->userData = vDev;

    bacnet_mac_set_uint16(&vDev->virtualMACaddr, vMAClocal);

    // just a note: remember that the pDev is attached to the datalink above..
    if (!ll_Enqueue(&frp->datalinkDevicesVirtual, vDev)) {
        panic();
        emm_free_two(vDev, pDev);
        return NULL;
    }

    return vDev;
}


VirtualDeviceInfo* Create_Device_Virtual(const uint8_t portId, const unsigned devInstance, const char* devName, const char* devDesc, const unsigned vendorId, const char* vendorName)
{
    static uint16_t vMAC = 0x0300;

    return Create_Device_Virtual_VMAC(portId, devInstance, devName, devDesc, vendorId, vendorName, vMAC++ );
}


#if 0
void Device_Init_Virtual(
    BPT_TYPE func,
    PORT_SUPPORT* datalink,
    DEVICE_OBJECT_DATA* pDev,
    uint32_t deviceInstance,
    const char* deviceName,
    const char* deviceDescription,
    const char* modelName,
    const void* userDeviceData
)
{

    Device_Init_Server(func, datalink, pDev, deviceInstance, deviceName, deviceDescription, modelName, userDeviceData);

    switch (func) {
    case BPT_APP:
    case BPT_BBMD:
    case BPT_BIP:
#if (BACDL_FD == 1)
    case BPT_FD:
#endif
        // only allowed one device on app datalink..
        if (pDev->datalink->datalinkDevices2.count != 0) {
            // uh oh, not allowed to create a second device on app datalink.
            panic();
            return;
        }
        break;
    case BPT_VIRT:
        // Are we trying to create a duplicate?
        if (Device_Find_VirtualDevice(deviceInstance) != NULL) return;
        ++vMAC;
        break;
    default:
        panic();
        return;
    }

    VirtualDeviceInfo* vDev = (VirtualDeviceInfo*)emm_calloc(sizeof(VirtualDeviceInfo));
    if (vDev == NULL)
    {
        panic();
        return;
    }
    bacnet_mac_set_uint16(&vDev->virtualMACaddr, vMAC);
    vDev->pDev = pDev;

    if (!ll_Enqueue(&datalink->datalinkDevices2, vDev))
    {
        panic();
        emm_free(vDev);
        return;
    }

    // now, what all the fuss is about, the VIRTUAL datalink (if used) will require knowledge about what the MAC address of the
    // device is that it is going to send a virtual packet from the virtual device to the virtual router. This is normally set up
    // when a packet is 'received', but here, because this is an unsolicited message, we need to set it up explicitely first.


    /* broadcast an I-Am on startup */
    // No, not all datalinks may be set up. Announce existence later in AnnounceDevices();
    // Send_I_Am_Broadcast(pDev);
}
#endif

#endif // BACNET_ROUTING