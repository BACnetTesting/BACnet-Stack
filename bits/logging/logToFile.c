/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc.* to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to * the following conditions:
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

#ifdef _MSC_VER
#include <wtypes.h>     // for Mutex Handle
#endif
#include <stdio.h>
#include <time.h>

#include "logging.h"

#ifdef _MSC_VER
HANDLE hIOMutex;
#endif

void log_puts_toFile(const char *tbuf)
{
    time_t timeNow = time(NULL);
    tm *ltime = localtime(&timeNow);

    char timebuf[100];
    strftime(timebuf, sizeof(timebuf), "%D %R", ltime);

    char filename[200];
    strftime(filename, sizeof(filename), "BACnetInterface Log %F.log", ltime);

    #ifdef _MSC_VER
    if (hIOMutex == NULL) hIOMutex = CreateMutex(NULL, FALSE, NULL);
    WaitForSingleObject(hIOMutex, INFINITE);
#endif

    char nocrbuf[200];
    size_t len = min(strlen(tbuf), sizeof(nocrbuf)-1);
    int cursor = 0;
    for (int i = 0; i < len; i++)
    {
        if (tbuf[i] != '\r' && tbuf[i] != '\n')
        {
            nocrbuf[cursor++] = tbuf[i];
        }
    }
    nocrbuf[cursor++] = 0;

    FILE *fp = fopen(filename, "a");
    if (fp) {
        fprintf(fp, "%s - %s\n", timebuf, nocrbuf );
        fclose(fp);
    }

#ifdef _MSC_VER
    ReleaseMutex(hIOMutex);
#endif
}


