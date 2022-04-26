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

#if ( BACNET_CLIENT == 1 )

#include "menuDiags.h"
#include "eLib/util/emm.h"
#include "eLib/util/llist.h"
#include "bacnet/basic/object/device.h"
#include "eLib/util/logging.h"
#include "bacnet/bits/util/BACnetToString.h"
// 2021-02-02 obsoleted #include "debug.h"
#include "clientSideHandlers.h"

#include "bitsDebug.h"

extern LLIST_HDR menuStack;

static ClientSidePollRecordCB* displayPollRecord ;

#if ( BACNET_CLIENT == 1 )
extern LLIST_HDR clientPollRecords;
#endif

static void ShowPollRecord(ClientSidePollRecordCB* currentPollRecord) {

    dbMessage(DBD_UI, DB_ALWAYS, "   Dev:%07d  PB:%s  Int:%d  Rem:%d",
        currentPollRecord->clientSideDevice->pDev->bacObj.objectInstance,
        characterstring_value(&currentPollRecord->bacnetObject->objectName),
        currentPollRecord->interval,
        currentPollRecord->nextPoll - time(NULL));
}

static void ShowNextItem(void) {
    if (displayPollRecord != NULL) displayPollRecord = (ClientSidePollRecordCB*)ll_Next(&clientPollRecords, displayPollRecord);
    if (displayPollRecord == NULL) displayPollRecord = (ClientSidePollRecordCB*)ll_First(&clientPollRecords);
    if (displayPollRecord == NULL) return;

    ShowPollRecord(displayPollRecord);
}



static void NextItem (void)
{
    ShowNextItem();
}


static bool HandleMenuKey(int key) {

    bool keyHandled = true;

    switch (key) {
    default:
        keyHandled = false;
        break;
    }
    return keyHandled;
}


static void HelpScreen(void) {
    log_printf("Poll Record");
}


static void EnterItem(void)
{
    ShowNextItem();
}


static void ExitMenu(void)
{
    // no cleanup required
}


void dm_PollRecord_Setup(int keyOnEnter )
{
    // if there is nothing to show, go no further
    if (clientPollRecords.count == 0)
	{
        log_printf("No items to show");
        return;
    }


    ACTIVE_MENU* am = (ACTIVE_MENU*)emm_calloc(sizeof(ACTIVE_MENU));
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

#endif // ( BACNET_CLIENT == 1 )
