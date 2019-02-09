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

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#ifdef _MSC_VER
#include <winsock2.h>
#endif
#include "osLayer.h"

bool CommandLineParameterExists(const int argc, char **argv, const char *parameter);
char *CommandLineParameter(const int argc, char **argv, const char *parameter);
char bits_toupper(const char orig);
size_t bits_strlen(const char *orig);
void CreateTestConfiguration(void);
void bits_memcpy(void *dest, const void *src, uint len);
void bits_memset(void *dest, uint8_t val, uint len);

//todo1
//void ip_address_parse(
//    BACNET_GLOBAL_ADDRESS * dst,
//    int argc,
//    char *argv[]) ;

// Use string_ToIPEP
//bool ipep_address_parse(
//		struct sockaddr_in *ipep,
//    const char *string);

char *ipep_address_sprintf(
    char *string,
	struct sockaddr_in *ipep);

// no longer void Device_Tables_Init(void);

void InitBACnet(void);
bool TickBACnet(void);

bool isMatchCaseInsensitive(const char *stringA, const char *stringB);

bool IsSubstring(
    const char* source,
    const char* substring);

void msSleep(uint16_t ms);

// moved to logging.h
//bool bitsKBhit(void);
//int bitsGetch(void);

uint32_t timer_get_time(void);
uint16_t timer_delta_time(uint32_t startTime);

bool doUserMenu(void);
void ShowMenuHelp(void);
void ShowCommandlineHelp(void);

void AnnounceDevices(void);

