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

#include "menuDiags.h"
#include "eLib/util/emm.h"
#include "eLib/util/llist.h"
#include "bacnet/basic/object/device.h"
#include "eLib/util/logging.h"
#include "bacnet/bits/util/BACnetToString.h"

extern LLIST_HDR menuStack;

extern PORT_SUPPORT* datalinkSupportHead;
static PORT_SUPPORT* displayDatalink;

#if ( BACNET_CLIENT == 1 )
void print_address_cache(PORT_SUPPORT* datalink);
#endif

static void ShowDatalink(PORT_SUPPORT* port) {
    dbMessage(DBD_UI, DB_ALWAYS, "   Datalink    ID:%-3d  Type:%-4s  Iface:%-8s  IPport:%5d",
        port->datalinkId,
        BPT_ToString(port->portType),
        port->ifName,
        ntohs(port->datalink.bipParams.nwoPort));
}


static void ShowNextItem(void) {
    if (displayDatalink != NULL) displayDatalink = (PORT_SUPPORT*)displayDatalink->llist.next ;
    if (displayDatalink == NULL) displayDatalink = datalinkSupportHead ;
    if (displayDatalink == NULL) return;

    ShowDatalink(displayDatalink);
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
        print_address_cache(displayDatalink);
        break;
#endif

    default:
        keyHandled = false;
        break;
    }
    return keyHandled;
}


static void HelpScreen(void) {
    log_printf("Datalinks");
    log_printf("   A)  Address Binding Cache");
}


static void EnterItem(void)
{
    ShowNextItem();
}


static void ExitMenu(void)
{
    // no cleanup required
}


void dm_Datalink_Setup(int keyOnEnter )
{
    // if there is nothing to show, go no further
    if (datalinkSupportHead == NULL ) return;

    ACTIVE_MENU* am = (ACTIVE_MENU*)emm_calloc(1,sizeof(ACTIVE_MENU));
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

