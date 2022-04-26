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

#include "osLayer.h"

#ifdef OS_LAYER_WIN
#include <winsock2.h>
#endif

bool CommandLineParameterExists(const int argc, char **argv, const char *parameter);
char *CommandLineParameter(const int argc, char **argv, const char *parameter);
bool CommandLineParameterCharExists(const int argc, char** argv, const char charParam);
char* CommandLineParameterChar(const int argc, char** argv, const char charParam);

char bits_toupper(const char orig);
size_t bits_strlen(const char *orig);
size_t bits_strlcpy(char *dest, const char *orig, const uint destLen);
void Create_Test_Configuration(const char* ifname);
// BACnet is _very_ specific about sizes (8,16,32 or more), so we try and make our
// variables reflect the BACnet uses of the same, so we don't want to use the 
// generic C types, int, uint, size_t
// for memory buffers, we use uint16_t....
void bits_memcpy(void *dest, const void *src, const uint16_t destLen);
void bits_memset(void *dest, const uint8_t val, const uint16_t destLen);
bool parse_command_line(int argc, char *argv[]);
char* bits_itoa(int value, char* str, int radix);

#define BITS_MIN(x, y) (((x) < (y)) ? (x) : (y))

char *ipep_address_sprintf(
    char *string,
	struct sockaddr_in *ipep);

// no longer void Device_Tables_Init(void);

bool isMatchCaseInsensitive(const char *stringA, const char *stringB);

bool IsSubstring(
    const char* source,
    const char* substring);

void msSleep(uint16_t ms);

char **Tokenize(char *line, const char divider, int *outCount);

bool bits_FileExists(const char* name);



