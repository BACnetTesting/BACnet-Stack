/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
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

#include "eLib/util/eLibUtil.h"

// does the command line parameter exist?
bool CommandLineParameterExists(const int argc, char** argv, const char* parameter)
{
    if (argc < 2) {
        return false;
    }

    for (int i = 1; i < argc; i++) {
        if (bits_strlen(argv[i]) > 1 &&
            argv[i][0] == '-' &&
            bits_toupper((int)argv[i][1]) == bits_toupper((int)* parameter)) {
            return true;
        }
    }
    return false;
}


bool CommandLineParameterCharExists(const int argc, char** argv, const char charParam)
{
    if (argc < 2) {
        return false;
    }

    for (int i = 1; i < argc; i++) {
        if (bits_strlen(argv[i]) > 1 &&
            argv[i][0] == '-' &&
            bits_toupper((int)argv[i][1]) == bits_toupper((int)charParam)) {
            return true;
        }
    }
    return false;
}


// returns pointer to the arg following the parameter string - If it exists. Else NULL (reason we can't use it for the above function)
char* CommandLineParameterChar(const int argc, char** argv, const char charParam)
{
    if (argc < 3) {
        return NULL;
    }

    for (int i = 1; i < argc ; i++) {
        if (strlen(argv[i]) > 1 &&
            argv[i][0] == '-' &&
            argv[i][1] == charParam )
        {
            if (argc > i) return argv[i + 1];
        }
    }
    return NULL;
}


char* CommandLineParameter(const int argc, char** argv, const char* parameter)
{
    if (argc < 3) {
        return NULL;
    }

    for (int i = 1; i < argc - 1; i++) {
        if (strlen(argv[i]) > 1 &&
            argv[i][0] == '-' &&
            isMatchCaseInsensitive(&argv[i][1], parameter))
        {
            if (argc > i) return argv[i + 1];
        }
    }
    return NULL;
}




