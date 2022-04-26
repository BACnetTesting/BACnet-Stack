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
extern LLIST_HDR serverDeviceCB;

DEVICE_OBJECT_DATA* displayDev;

static void ShowBACnetServerDevice(DEVICE_OBJECT_DATA* pDev) 
{
#if 0
    dbMessage(DBD_UI, DB_ALWAYS, "      DevID:%-7d  Name:%-40s  AI:%d AO:%d AV:%d  BI:%d BO:%d BV:%d  MSI:%d MSO:%d MSV:%d    %c         ",
        pDev->bacObj.objectInstance,
        characterstring_value(&pDev->bacObj.objectName),
        pDev->AI_Descriptor_List.count,
        pDev->AO_Descriptor_List.count,
        pDev->AV_Descriptor_List.count,
        pDev->BI_Descriptor_List.count,
        pDev->BO_Descriptor_List.count,
        pDev->BV_Descriptor_List.count,
#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
        pDev->MSI_Descriptor_List.count,
        pDev->MSO_Descriptor_List.count,
        pDev->MSV_Descriptor_List.count,
#else
        0,0,0,
#endif
#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )
        ( pDev->TSM_List == NULL ) ? ' ' : 'T'
#else
        'N'
#endif
    );
#endif
}

static void ShowNextDevice(void) {
    if (displayDev != NULL) displayDev = (DEVICE_OBJECT_DATA*)ll_Next(&serverDeviceCB, displayDev);
    if (displayDev == NULL) displayDev = (DEVICE_OBJECT_DATA*)ll_First(&serverDeviceCB);

    ShowBACnetServerDevice(displayDev);
}

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )

static void SetupTSMmenu(void) {
    log_printf("TSM Table for this dev");
    for (int i = 0; i < MAX_TSM_TRANSACTIONS; i++) {

        BACNET_TSM_DATA* tsm = &displayDev->TSM_List[i];
        if (tsm == NULL) {
            // presumably not a device that needs a TSM... todo 2 we should check this fact later
            continue;
        }

        if (tsm->state != TSM_STATE_IDLE) {
            log_printf("      %3d %2d %5d/%s",
                i,
                tsm->state,
                tsm->dlcb2->bacnetPath.glAdr.net,
                BACnetMacAddrToString(&tsm->dlcb2->bacnetPath.localMac)
            );
        }
    }
}
#endif


static void NextItem (void)
{
    ShowNextDevice();
}


static bool HandleMenuKey(int key) {

    bool keyHandled = true;

    switch (key) {
#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )
    case 'T':
        SetupTSMmenu();
        break;
#endif
    default:
        keyHandled = false;
        break;
    }
    return keyHandled;
}


static void HelpScreen(void) {
    log_printf("Server Device Menu");
    log_printf("   T)  Show TSM Table");
}


static void EnterItem(void)
{
    ShowNextDevice();
}


static void ExitMenu(void)
{
    // no cleanup required
}


void dm_ServerDevice_Setup(int keyOnEnter )
{
    // if there is nothing to show, go no further
    if (serverDeviceCB.count == 0) return;

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

    ShowNextDevice();
}
