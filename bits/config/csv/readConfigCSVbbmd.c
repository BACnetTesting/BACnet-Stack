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

#include "osLayer.h"
#include "emm.h"
#include "device.h"
#include "net.h"
#include "bbmd.h"
//#include "bitsUtil.h"
#include "bitsDebug.h"
#include "BACnetToString.h"
#include "readConfigCSVbbmd.h"

#define MX_TOKENS   10 

char **Tokenize(char *line, int *outCount)
{
    if (line[0] == 0) {
        *outCount = 0;
        return NULL;
    }

    int size = MX_TOKENS * sizeof(char *);
    char **tokens = (char **)emm_calloc(size);

    // blunt instrument....
    int len = strlen(line) - 1;
    int index = 0;
    tokens[index++] = &line[0];
    for (int i = 1; i < len; i++) {
        if (line[i] == ',') {
            line[i] = 0;
            if (line[i + 1] != 0) {
                tokens[index++] = &line[i + 1];
            }
            i++;
        }
    }
    *outCount = index;
    return tokens;
}


void ParseBBMDfile( const char *bbmdFileName )
{
    FILE *configFile;
    char line[1024];
    BBMD_TABLE_ENTRY BBMD_Table_Entry;
    char *address = NULL;
    char *mask = NULL;

    configFile = fopen(bbmdFileName, "r");

    if (configFile == NULL) {
        dbTraffic(DBD_ALL, DB_BTC_ERROR, "Failed to open BBMD config file '%s'", bbmdFileName);
        return;
    }
    else {
        printf("******* BACnet Gateway BBMD Settings *******\n");

        while (!feof(configFile)) // read through the file until End of File
        {
            int count;
            SOCKADDR_IN ipep;
            fgets(line, 1024, configFile); // grab one line from the file at a time

            char **tokens = Tokenize(line, &count);
            if (tokens != NULL) {
                if (!StringTo_IPEP(&ipep, tokens[0])) {
                    panic();
                    emm_free(tokens);
                    continue;
                }
                emm_free(tokens);

                memset(&BBMD_Table_Entry, 0, sizeof(BBMD_TABLE_ENTRY));
                BBMD_Table_Entry.valid = true;
                BBMD_Table_Entry.dest_address.s_addr = ipep.sin_addr.s_addr ;
                BBMD_Table_Entry.dest_port = ipep.sin_port ;
                BBMD_Table_Entry.broadcast_mask.s_addr = 0xFFFFFFFF ;
                bvlc_add_bdt_entry_local(&BBMD_Table_Entry);
            }
            else {
                panic();
            }
        }

        printf("******* BACnet Gateway BBMD Settings *******\n\n");

        fclose(configFile);
    }
}


