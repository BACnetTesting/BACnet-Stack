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

#include "bitsUtil.h"
#include "tokenize.h"
#include "emm.h"
#include "bitsDebug.h"

#define MX_TOKENS   50 

char **Tokenize(char *line, uint *outCount)
{
    if (line[0] == 0) {
        *outCount = 0;
        return NULL;
    }
    
    int size = MX_TOKENS * sizeof(char *);
    char **tokens = (char **)emm_calloc(size);

    // blunt instrument....
    uint len = bits_strlen(line);
    if (len == 0)
    {
        panicDesc("Cannot tokenize a string of length 0");
        *outCount =  0;
        return NULL ;
    }
    len--;
    uint index = 0;
    tokens[index++] = &line[0];
    for (int i = 1; i < len; i++) {
        if (line[i] == ',') {
            line[i] = 0;
            if (line[i + 1] != 0) {
                tokens[index++] = &line[i + 1];
            }
            i++;
        }
        if (line[i] == '\r' || line[i] == '\n')
        {
            // we are done with this line
            line[i] = 0;
            break;
        }
    }
    *outCount = index;
    return tokens;
}
