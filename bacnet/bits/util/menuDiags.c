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

#ifdef OS_LAYER_WIN
#include <conio.h>      // kbhit - for now
#endif 
#include "logging/logging.h"
#include "bitsUtil.h"
#include "osLayer.h"
#include "BACnetToString.h"
#include "bitsDebug.h"
#if defined(BBMD_ENABLED) && BBMD_ENABLED
#include "bbmd.h"
#endif

bool showIPChandshake;

#if defined(BBMD_ENABLED) && BBMD_ENABLED
extern BBMD_TABLE_ENTRY BBMD_Table[MAX_BBMD_ENTRIES];
extern FD_TABLE_ENTRY FD_Table[MAX_FD_ENTRIES];
extern struct sockaddr_in Remote_BBMD;
#endif

#if defined(BBMD_ENABLED) && BBMD_ENABLED
static void ShowBDT(void)
{
    bool entries = false;
    log_printf("BBMD Device Table:");
    for (int i = 0; i < MAX_BBMD_ENTRIES; i++) {
        BBMD_TABLE_ENTRY *bte = &BBMD_Table[i];
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
    if ( Remote_BBMD.sin_addr.s_addr )
    {
        char tstring[100];
        log_printf("   %s", IPEP_ToString(tstring, &Remote_BBMD ));
    }
    else {
        log_printf("   No entries.");
    }

    log_printf("Foreign Device Table:");
    for (int i = 0; i < MAX_FD_ENTRIES; i++) {
        FD_TABLE_ENTRY *bte = &FD_Table[i];
        if (bte->dest_address.s_addr) {
            entries = true;
             char tstring[100];
            log_printf("   %s,  TTL:%d", IPAddr_Port_ToString(tstring, &bte->dest_address, bte->dest_port), bte->time_to_live);
        }
    }
    if (!entries) log_printf("   No entries.");
}
#endif

static void ShowNextObject(void)
{
    log_printf("Todo");
}


void ShowMenuHelp(void)
{
    log_printf("");
    log_printf("Keys: Q)uit");
    log_printf("      O) Next Object");
    log_printf("      E) EMM trace log");
#if ( BITS_USE_IPC == 1 )
    log_printf("      I) Show IPC handshake (%s)", showIPChandshake ? "On" : "Off" );
#endif
    log_printf("      A) Address cache   T)SM cache");
    log_printf("      C) Create Test Configuration");
#if defined(BBMD_ENABLED) && BBMD_ENABLED
    log_printf("      B) Show BDT       F) Show FDT");
#endif
    log_printf("");
}


bool doUserMenu(void)
{

    // get a key and post it to redirected BITS input if appropriate
    if (osKBhit())
        bits_KeyInput(osGetch());

    if (bits_kbhit()) {

        switch (bits_getch_toupper()) {

#if defined(BBMD_ENABLED) && BBMD_ENABLED
        case 'B':
            ShowBDT();
            break;

        case 'F':
            ShowFDT();
            break;
#endif

        case 'C':
            panic();
            // CreateTestConfiguration();
            log_printf("Objects created");
            break;

#if ( BITS_USE_IPC == 1 )
        case 'I':
            showIPChandshake = !showIPChandshake;
            ShowMenuHelp();
            break;
#endif

        case 'L':
            break;
        case 'O':
            ShowNextObject();
            break;
        case 'Q':
            return false;
        default:
            ShowMenuHelp();
            break;
        }
    }
    return true;
}




