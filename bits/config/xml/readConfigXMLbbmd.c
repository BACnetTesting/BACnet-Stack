// bah, it does not make sense to use xml format for this (just yet. see configSimple..)

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

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//// #include <BACnetObjectInterface.h>
//// #include "readconfig.h"
#include "device.h"
//#include "net.h"
//#include "bip.h"
#include "bitsUtil.h"
#include "bitsDebug.h"
#include "debug.h"

//#ifndef _MSC_VER
//#include "indtext.h"    // for manual stricmp() for vxWorks
//#endif

using namespace std;

#define MSTPstring "MSTP"

extern uint32_t Object_Instance_Number;
extern bool enableBTA;

extern char Description[MAX_DEV_DESC_LEN + 1];      // doubles for Model Name in this implementation..

char DeviceObjectName[MAX_DEV_DESC_LEN+1];

#ifdef _MSC_VER
// char *settingsFilename;
char *bbmdFilename;
#else
char *bbmdFilename = BBMD_FILENAME;
#endif

char *XMLStringPull(char *line, char *startTag, char *endTag)
{
    // kctodo, after clearing the magic number (ie 1024) make sure that this function does not return a string
    // larger than that size

    // future, use C++ strings instead

    char *temp1;
    char *temp2;
    char *temp3;

    temp1 = strstr(line, startTag); // find the start tag
    if (temp1 == NULL) // if not found
    {
        printf("Tag: '%s' not found in Line: '%s'\n", startTag, line);
        return line; // return original string
    }
    temp2 = strstr(line, startTag);
    if (temp2 == NULL) {
        printf("Tag: '%s' not found in Line: '%s'\n", endTag, line);
        return line; // return the original string
    }
    temp2 += strlen(startTag); // capture pointer to original string after start tag
    temp3 = strstr(temp1, endTag); // find end tag from this string

    if (temp3 == NULL) // if end tag not found
    {
        printf("Tag: '%s' not found in Line: '%s'\n", endTag, line);
        return temp2; // return the truncated string
    }
    temp3[0] = '\0'; // otherwise, clip the string at the location where the end tag was found

    return temp2; // and return this string for use
}


void parseFPMConfigFile()
{
    FILE *configFile;
    char line[1024];
    char *address = NULL;
    char *mask = NULL ;

    // MSTP Settings
    char strBBMDaddressStart[] = "<bbmdAddress>";
    char strBBMDaddressEnd[] = "</bbmdAddress>";
    char strBroadcastDistrubutionMaskStart[] = "<broadcastDistrubutionMask>";
    char strBroadcastDistrubutionMaskEnd[] = "</broadcastDistrubutionMask>";

    configFile = fopen( bbmdFilename, "r");

    if(configFile != NULL)
    {
        dbTraffic(DB_NOTE, "Failed to open config file '%s'", bbmdFilename);
    }
    else
    {
        printf("******* BACnet Gateway BBMD Settings *******\n");

        while (!feof(configFile)) // read through the file until End of File
        {
            fgets(line, 1024, configFile); // grab one line from the file at a time

            if (IsSubstring(line, strBBMDaddressStart))
            {
                address = XMLStringPull(line, strBBMDaddressStart, strBBMDaddressEnd);
            }
            else if (IsSubstring(line, strBroadcastDistrubutionMaskStart))
            {
                mask = XMLStringPull(line, strBroadcastDistrubutionMaskStart, strBroadcastDistrubutionMaskEnd);
            }

            if (mask == NULL) mask = "255.255.255.255";
            if (address == NULL)                 

            }
        }

        printf("******* BACnet Gateway BBMD Settings *******\n\n");

        fclose(configFile);

    }
}


void set_device_object_name(char *deviceObjectName)
{
    BACNET_CHARACTER_STRING char_string;
    characterstring_init_ansi(&char_string, deviceObjectName);
    Device_Set_Object_Name(&char_string);
}
