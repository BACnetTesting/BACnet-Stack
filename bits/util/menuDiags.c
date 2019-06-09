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

// #include <ctype.h>      // for toupper
#include <conio.h>      // kbhit - for now
#include "logging/logging.h"
#include "bitsUtil.h"
#include "osLayer.h"
// #include "BACnetToString.h"
#include "bitsDebug.h"

bool showIPChandshake;

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
    log_printf("      I) Show IPC handshake (%s)", showIPChandshake ? "On" : "Off" );
    log_printf("      A) Address cache   T)SM cache");
    log_printf("      C) Create Test Configuration");
    log_printf("");
}


bool doUserMenu(void) {

    // get a key and post it to redirected BITS input if appropriate
    if (osKBhit())
        bits_KeyInput(osGetch());

    if (bits_kbhit()) {

        switch (bits_getch_toupper()) {

        case 'C':
            panic();
            // CreateTestConfiguration();
            log_printf("Objects created");
            break;
        case 'I':
            showIPChandshake = !showIPChandshake;
            ShowMenuHelp();
            break;
        case 'L':
//            ShowDatalinks();
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




