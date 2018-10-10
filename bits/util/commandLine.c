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

// does the command line parameter exist?
bool CommandLineParameterExists(const int argc, char **argv, const char *parameter)
{
    if (argc < 2) {
        return false;
    }

    for (int i = 1; i < argc; i++) {
        if ( bits_strlen(argv[i]) > 1 &&
                argv[i][0] == '-' &&
                bits_toupper((int) argv[i][1]) == bits_toupper( (int) *parameter)) {
            return true;
        }
    }
    return false ;
}


// returns pointer to the arg following the parameter string - If it exists. Else NULL (reason we can't use it for the above function)
char *CommandLineParameter(const int argc, char **argv, const char *parameter)
{
    if (argc < 3) {
        return NULL ;
    }

    for (int i = 1; i < argc-1; i++) {
        if (strlen(argv[i]) > 1 &&
            argv[i][0] == '-' &&
            isMatchCaseInsensitive(&argv[i][1], parameter))
        {
            if (argc > i) return argv[i + 1];
        }
    }
    return NULL ;
}


//bool ipep_address_parse(
//    SOCKADDR_IN *ipep,
//    const char *string )
//{
//    //unsigned mac[6];
//    //unsigned port;
//    //int count = 0;
//
//    //count = sscanf( string, "%u.%u.%u.%u:%u", &mac[0], &mac[1], &mac[2],
//    //            &mac[3], &port);
//    //        
//    // if (count != 5) return false ;
//    // 
//    // ipep->sin_addr.S_un.S_un_b.s_b1 = mac[0];
//    // ipep->sin_addr.S_un.S_un_b.s_b2 = mac[1];
//    // ipep->sin_addr.S_un.S_un_b.s_b3 = mac[2];
//    // ipep->sin_addr.S_un.S_un_b.s_b4 = mac[3];
//
//    // ipep->sin_port = htons(port);
//
//    // return true;
//    return StringTo_IPEP(ipep, string);
//}

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

