/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#include <ctype.h>      // for toupper
#include "logging.h"
#include "bitsUtil.h"
// #include "BACnetToString.h"

static void ShowNextObject(void)
{
}

void ShowMenuHelp(void)
{
    log_printf("");
    log_printf("Keys: Q)uit");
    log_printf("      O) Next Object");
    log_printf("      E) EMM trace log");
    log_printf("      A) Address cache   T)SM cache");
    log_printf("");
}

bool doUserMenu(void) {
    if (bitsKBhit()) {
        int ch = bitsGetch();
        switch (tolower(ch)) {
        case 'o':
            ShowNextObject();
            break;
        case 'q':
            return false;
        default:
            ShowMenuHelp();
            break;
        }
    }
    return true;
}




