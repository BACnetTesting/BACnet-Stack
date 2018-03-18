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

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#ifdef _MSC_VER
#include <winsock2.h>
#endif

bool CommandLineExists(const int argc, char **argv, const char *parameter);
char *CommandLineParameter(const int argc, char **argv, const char *parameter);

//todo1
//void ip_address_parse(
//    BACNET_GLOBAL_ADDRESS * dst,
//    int argc,
//    char *argv[]) ;

bool ipep_address_parse(
		struct sockaddr_in *ipep,
    const char *string);

char *ipep_address_sprintf(
    char *string,
	struct sockaddr_in *ipep);

// no longer void Device_Tables_Init(void);

namespace bits
{
    void InitBACnet(void);
    bool TickBACnet(void);
}

bool isMatchCaseInsensitive(const char *stringA, const char *stringB);
void msSleep(uint16_t ms);
bool bitsKBhit(void);
int bitsGetch(void);

uint32_t timer_get_time(void);
uint16_t timer_delta_time(uint32_t startTime);

bool doUserMenu(void);
void ShowMenuHelp(void);

void AnnounceDevices(void);


