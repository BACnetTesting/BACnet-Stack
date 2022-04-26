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
#include "bitsDebug.h"
#include "BACnetToString.h"
#include "readConfigCSVbbmd.h"
#include "bactext.h"
#include "appApi.h"

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


void ParseCSVfile(const char *fileName, void(*callback)(char **tokens, const uint count, const void *aParameter ), const uint32_t deviceInstance )
{
    FILE *csvFile = fopen(fileName, "r");
    if (csvFile == NULL)
    {
        dbTraffic(DBD_Config, DB_ERROR, "Could not open CSV file [%s]", fileName);
        return ;
    }
    
    while (!feof(csvFile)) 
    {
        char line[1024];
        int outCount;
        fgets(line, 1024, csvFile);  // grab one line from the file at a time
        if(!feof(csvFile))
        {
        char **tokens = Tokenize(line, &outCount);
        if (outCount > 0)
        {
            callback( tokens, outCount, (const void *) &deviceInstance );
        }
        emm_free(tokens);
        }
    }
    
    fclose(csvFile);
}


void TemplateCallback(char **tokens, const uint count, const void *aParameter )
{
    uint32_t *devInst = (uint32_t *) aParameter;
    
    for (int i = 0; i < count; i++) 
    {
        printf("item %d = %s\n", i, tokens[i]);
    }

    BACNET_OBJECT_TYPE found_type;
    bool status = bactext_object_type_enum_by_acronym(tokens[1], &found_type);
    if (!status) 
    {
        dbTraffic(DBD_Config, DB_ERROR, "Unknown BACnet Type [%s]", tokens[1] );
        return ;
    }
    
    IPC_OBJECT_TYPE ipcType = BACapi_BACnet_to_IPC_Type(found_type);
    if (ipcType == OBJ_TYPE_Error )
    {
        dbTraffic(DBD_Config, DB_ERROR, "Unknown IPC Type [%s]", tokens[1] );
        return ;
    }
    
    uint objInst = atoi(tokens[2]);
    
    Create_Object( *devInst, ipcType, objInst, tokens[3], tokens[4] );
    
}


void ParseTemplateFile(const char *modelName, const uint32_t deviceInstance )
{
    char fullname[1000];
    // todo 0 strcpy(fullname, "/var/apollo/data/bacnetrouter/template_");
    strcpy(fullname, "../app/Adesto/ClientServerRouter/resource/template_");
    strcat(fullname, modelName);
    strcat(fullname, ".txt");
    
    ParseCSVfile(fullname, TemplateCallback, deviceInstance );
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

void ParseFDRfile(const char *fileName)
{
    FILE *configFile;
    char line[1024];
    char *address = NULL;
    char *mask = NULL;

    configFile = fopen(fileName, "r");

    if (configFile == NULL)
    {
        dbTraffic(DBD_ALL, DB_BTC_ERROR, "FDR config filename %s does not exist. Ignoring Foreign Device Registration.\n", fileName);
        return;
    }
    else
    {
        dbTraffic(DBD_ALL, DB_BTC_ERROR, "******* BACnet Gateway FDR Settings *******\n");

        while (!feof(configFile)) // read through the file until End of File
        {
            int count;
            struct sockaddr_in ipep;
            fgets(line, 1024, configFile); // grab one line from the file at a time

            char **tokens = Tokenize(line, &count);
            if (tokens != NULL && strlen ( tokens[0] ) > 9)
            {
                if (!StringTo_IPEP(&ipep, tokens[0]))
                {
                    dbTraffic(DBD_ALL, DB_BTC_ERROR, "Error, could not parse: %s\n", line);
                    // panic();
                    // emm_free(tokens);
                    continue;
                }
                free(tokens);

                char tbuf[30];
                dbTraffic(DBD_ALL, DB_BTC_ERROR, "Will register with BBMD %s\n", IPEP_ToString( tbuf, &ipep) );
                dbTraffic(DBD_ALL, DB_BTC_ERROR, "Need to add BBMD support");
                // bbmd_address = ipep.sin_addr.s_addr;
                // bbmd_port = ntohs( ipep.sin_port ) ;
            }
            else
            {
                // panic();
            }
        }

        dbTraffic(DBD_ALL, DB_BTC_ERROR, "******* BACnet Gateway FDR Settings *******\n\n");

        fclose(configFile);
    }
}

