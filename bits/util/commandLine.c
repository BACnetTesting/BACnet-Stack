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

#include <stdbool.h>
#include <stdio.h>
#ifdef __GNUC__
#include <ctype.h>		// for toupper()
#endif
#include <string.h>
//#include <winsock2.h>
#include "BACnetToString.h"

// does the command line parameter exist?
bool CommandLineExists(const int argc, char **argv, const char *parameter)
{
    if (argc < 2) {
        return false;
    }

    for (int i = 1; i < argc; i++) {
        if ( strlen(argv[i]) > 1 &&
                argv[i][0] == '-' &&
                toupper((int) argv[i][1]) == toupper( (int) *parameter)) {
            return true;
        }
    }
    return false ;
}

// returns pointer to the arg following the parameter string 

char *CommandLineParameter(const int argc, char **argv, const char *parameter)
{
    if (argc < 3) {
        return NULL ;
    }

    for (int i = 1; i < argc-1; i++) {
        if (strlen(argv[i]) > 1 &&
            argv[i][0] == '-' &&
#ifdef _MSC_VER
            !_strcmpi(&argv[i][1], parameter)) 
#else
            !strcmp(&argv[i][1], parameter))
#endif
        {
            if (argc > i) return argv[i + 1];
        }
    }
    return NULL ;
}


bool ipep_address_parse(
    SOCKADDR_IN *ipep,
    const char *string )
{
    //unsigned mac[6];
    //unsigned port;
    //int count = 0;

    //count = sscanf( string, "%u.%u.%u.%u:%u", &mac[0], &mac[1], &mac[2],
    //            &mac[3], &port);
    //        
    // if (count != 5) return false ;
    // 
    // ipep->sin_addr.S_un.S_un_b.s_b1 = mac[0];
    // ipep->sin_addr.S_un.S_un_b.s_b2 = mac[1];
    // ipep->sin_addr.S_un.S_un_b.s_b3 = mac[2];
    // ipep->sin_addr.S_un.S_un_b.s_b4 = mac[3];

    // ipep->sin_port = htons(port);

    // return true;
    return StringTo_IPEP(ipep, string);
}

//char *ipep_address_sprintf(
//    char *string,
//    SOCKADDR_IN *ipep )
//{
//    sprintf(string, "%03u.%03u.%03u.%03u:%u",
//        ipep->sin_addr.S_un.S_un_b.s_b1,
//        ipep->sin_addr.S_un.S_un_b.s_b2,
//        ipep->sin_addr.S_un.S_un_b.s_b3,
//        ipep->sin_addr.S_un.S_un_b.s_b4,
//        ntohs(ipep->sin_port));
//    return string;
//}

