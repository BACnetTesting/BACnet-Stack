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

#include "configProj.h"

#if (BITS_ROUTER_LAYER == 1)

#include "menuDiags.h"
#include "eLib/util/emm.h"
#include "eLib/util/llist.h"
#include "bacnet/basic/object/device.h"
#include "eLib/util/logging.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"

extern LLIST_HDR menuStack;

extern ROUTER_PORT* headRouterPort;
extern ROUTER_PORT* applicationRouterPort;
extern ROUTER_PORT* associatedRouterPort;

static ROUTER_PORT* displayRouterPort;

bits_mutex_extern(bacnetStackMutex);

#if ( BACNET_CLIENT == 1 )
void print_address_cache(PORT_SUPPORT *datalink );
#endif

static void ShowRouterPort(ROUTER_PORT* port)
{
    char tbuf2[200];
    char assoc = ' ';

    BACNET_MAC_ADDRESS macaddr;

    if (port == associatedRouterPort)
    {
        assoc = 'A';
    }

    switch (port->port_support->portType )
    {
    case BPT_BIP:
    case BPT_BBMD:
#if (BACDL_FD == 1)
    case BPT_FD:
#endif    
        IPAddr_ToString(tbuf2, (const struct in_addr*)&port->port_support->datalink.bipParams.nwoBroadcast_addr);
        break;

#if (BACDL_MSTP == 1)
    case BPT_MSTP:
        strcpy(tbuf2, "255");
        break;
#endif

#if (BACDL_ETHERNET == 1)
    case BPT_Ethernet:
        strcpy(tbuf2, "FF-FF-FF-FF-FF-FF");
        break;
#endif

    case BPT_VIRT:
    case BPT_APP:
        strcpy(tbuf2, "N/A");
        break;
    default:
        panic();
    }

    port->port_support->get_MAC_address(port->port_support, &macaddr);

    dbMessage(DBD_UI, DB_ALWAYS, " %c RP ID:%3d  Type:%6s  BMAC: %-18s  Br: %-17s  NN:%5d  uxDev:%d  if: %-8s",
        assoc,
        port->port_id,
        BPT_ToString(port->port_support->portType),
        BACnetMacAddrToString(&macaddr),
        tbuf2,
        port->route_info.configuredNet.net,
        0, // write a function to calculate this count. //port->port_support->datalinkServerDevices.count,
        port->port_support->ifName);
}


void ShowRouterPorts(void)
{
    ROUTER_PORT* port = headRouterPort;

    bits_mutex_lock(bacnetStackMutex);

    dbMessage(DBD_UI, DB_ALWAYS, "Router Ports:");
    while (port) {
        ShowRouterPort(port);
        port = (ROUTER_PORT*)port->llist.next;
    }

    // and show the application port
    if (applicationRouterPort == NULL) {
        dbMessage(DBD_ALL, DB_ERROR, "Application RouterPort is not associated");
    }

    bits_mutex_unlock(bacnetStackMutex);
}


static void ShowNextItem(void) {
    if (displayRouterPort != NULL) displayRouterPort = (ROUTER_PORT*)displayRouterPort->llist.next ;
    if (displayRouterPort == NULL) displayRouterPort = headRouterPort;
    if (displayRouterPort == NULL) return;

    ShowRouterPort(displayRouterPort);
}


static void NextItem (void)
{
    ShowNextItem();
}


static bool HandleMenuKey(int key) {

    bool keyHandled = true;

    switch (key) {

#if ( BACNET_CLIENT == 1 )
    case 'A':
        print_address_cache(displayRouterPort->port_support );
        break;
#endif

    default:
        keyHandled = false;
        break;
    }
    return keyHandled;
}


static void HelpScreen(void) {
    dbMessage(DBD_UI, DB_ALWAYS, "RouterPorts");
    dbMessage(DBD_UI, DB_ALWAYS, "   A)  Address Binding Cache");
}


static void EnterItem(void)
{
    ShowNextItem();
}


static void ExitMenu(void)
{
    // no cleanup required
}


void dm_RouterPort_Setup(int keyOnEnter)
{
    // if there is nothing to show, go no further
    if (headRouterPort == NULL) 
	{
        log_printf("No items to show");
        return;
    }

    ShowRouterPorts();

    ACTIVE_MENU* am = (ACTIVE_MENU*)emm_calloc(1, sizeof(ACTIVE_MENU));
    if (am == NULL)
    {
        panic();
        return;
    }
    am->EnterItem = EnterItem;
    am->ExitMenu = ExitMenu;
    am->NextItem = NextItem;
    am->HelpScreen = HelpScreen;
    am->HandleKey = HandleMenuKey;

    am->keyOnEnter = keyOnEnter;

    ll_Enqueue(&menuStack, am);

    HelpScreen();

    ShowNextItem();
}

#endif // #if (BITS_ROUTER_LAYER == 1)
