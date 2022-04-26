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

#include "configProj.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// #include <conio.h>

//#include "bacstr.h"
//#include "osLayer.h"
//#include "device.h"
//#include "bacversion.h"
//#include "appApi.h"
//#include "bitsUtil.h"
//#include "logging.h"
#include "eLib/util/eLibDebug.h"
#include "eLib/util/eLibUtil.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"

// the following just so we can dummy linux optargs...
typedef enum
{
    required_argument,
    no_argument
} optionEnum ;

struct option {
    const char* name;
    optionEnum enumname;
    void* temp;
    char temp2;
};

#if ( BITS_ROUTER_LAYER == 1)
extern ROUTER_PORT *headRouterPort; /* pointer to list of router ports */
// extern ROUTER_PORT *applicationRouterPort;
#endif

bool hardcodedTestConfig;

bool logToConsole;
extern char cmdIfname[30]; //  = { "10.10.10.11" };
static char staticIfname[30];
extern uint16_t localPhysicalPort;

void ShowCommandlineHelp(void)
{
    //log_printf("   -e n     Debug lev, 1 = min, 9 most verbose");
    //log_printf("   -h       This message");
    //log_printf("   -i       Interactive console");
    //log_printf("   -t ens33 Use hardcoded test configuration, on given interface");
    //log_printf("   -v       Show Version");
}


bool parse_command_line(
    int argc,
    char *argv[])
{
    if (argc < 3)
    {
        printf("Specify your local IP address and BACnet Port on command line (or in debugger settings)." );
        osGetch();
        exit(1);
    }
    bits_strlcpy(cmdIfname, argv[1], sizeof(cmdIfname));
    localPhysicalPort = (uint16_t)atoi(argv[2]);

    // mutexDnet = CreateMutex(NULL, FALSE, NULL);
    // mutexLon = CreateMutex(NULL, FALSE, NULL);
    // mutexRingbuf = CreateMutex(NULL, FALSE, NULL);


    const char *optString = "rifhs:t:c::D:ve:g::";                // trailing : means option requires parameter, :: optional parameter
    // const char *bipString = "p:n:D:";
    const struct option Options[] = {
        { "config_file", required_argument, NULL, 'c' },
        { "device", required_argument, NULL, 'D' },
        { "verbosityLevel", required_argument, NULL, 'e' },
        { "ignore_parameter", required_argument, NULL, 'g' },
        { "interactive_mode", no_argument, NULL, 'i' },
        { "mac", required_argument, NULL, 'm' },
        { "network", required_argument, NULL, 'n' },
        { "port", required_argument, NULL, 'P' },
        { "reveal", no_argument, NULL, 'r' },
        { "test", no_argument, NULL, 't' },
        { "help", no_argument, NULL, 'h' },
        { NULL, no_argument, NULL, 0 },
    };

    int opt ;
    int dblev;
#if ( BITS_ROUTER_LAYER == 1)
    ROUTER_PORT *current = headRouterPort;
#endif
    char* optPtr = (char *) optString;
    int optind = 0;

    /* begin checking cmd parameters */
    do
    {
        char *optarg;

        if (*(optPtr + 1) == ':') {
            // trailing parameter
            optarg = CommandLineParameterChar(argc, argv, *optPtr);
            if (optarg == NULL) continue;
            opt = *optPtr;
        }
        else {
            optarg = NULL;
            if (!CommandLineParameterChar(argc, argv, *optPtr))
               {
                    continue;
               }
            opt = *optPtr;
        }

        // -ve opt signals end of options..
        if( *optPtr == 0 )
        {
            break;
        }

        switch (opt) {

        case 'i':
            logToConsole = true;
            break;

        case 'h':
            logToConsole = true;
            ShowCommandlineHelp();
            exit(-1);
            break;

        case 'e':
        {
            dblev = atoi(optarg);
            dbMessageSetLevel(dblev);
            int oldConsole = logToConsole;
            logToConsole = true;
            dbMessage(DBD_Config, DB_NOTE, "Debug level set to %d", dblev);
            logToConsole = oldConsole;
        }
            break;

        case 'g':
            // ignore trailing parameter
            break;

        case 'v':
            logToConsole = true;
            dbMessage(DBD_Config, DB_ALWAYS, "%s", "Version: " APPLICATION_VERSION_TEXT);
            exit(0);
            break;

        case 'c':
            dbMessage(DBD_Config, DB_NOTE, "-c parameter is now obsolete" );
            break;

#if ( BITS_ROUTER_LAYER == 1 )
        case 'D':

            /* create new list node to store port information */
            if (headRouterPort == NULL) {
                headRouterPort = (ROUTER_PORT *)malloc(sizeof(ROUTER_PORT));
                if (headRouterPort == NULL) return false;
                headRouterPort->llist.next = NULL;
                current = headRouterPort;
            }
            else {
                ROUTER_PORT *tmp = current;
                current = (ROUTER_PORT *) current->llist.next;
                current = (ROUTER_PORT *)malloc(sizeof(ROUTER_PORT));
                if (current == NULL) return false;
                current->llist.next = NULL;
                tmp->llist.next = (LLIST *) current;
            }

            // port_count++;
            if(strcmp(optarg, "bip") == 0)
            {
                if (optind < argc && argv[optind][0] != '-') {
                    strcpy((char *) current->port_support->ifName, argv[optind]);
                }
                else {
                    strcpy((char *) current->port_support->ifName, "eth0");
                }

                /* setup default parameters */
                current->port_support->datalink.bipParams.nwoPort = htons(0xBAC0); /* 47808 */
                current->route_info.configuredNet.net = get_next_free_dnet();

                dbMessage(DBD_Config, DB_NOTE, "Interface2: %s", current->port_support->ifName);

                //                  /* check if interface is valid */
                //                  fd = socket(AF_INET, SOCK_DGRAM, 0);
                //                  if (fd) {
                //                      struct ifreq ifr;
                //                      strncpy(ifr.ifr_name, current->iface,
                //                          sizeof(ifr.ifr_name) - 1);
                //                      result = ioctl(fd, SIOCGIFADDR, &ifr);
                //                      if (result != -1) {
                //                          close(fd);
                //                      } else {
                //                          PRINT(ERROR,
                //                              "Error: Invalid interface for BIP device 2");
                //                          return false;
                //                      }
                //                  }
#if 0
                dev_opt =
                    getopt_long(argc, argv, bipString, Options, &index);
                while (dev_opt != -1 && dev_opt != 'd') {
                    switch (dev_opt) {
                    case 'P':
                        result = atoi(optarg);
                        if (result) {
                            current->port_support->datalink.bipParams.nwoPort = htons((uint16_t)result);
                        }
                        else {
                            current->port_support->datalink.bipParams.nwoPort = htons((uint16_t) 0xBAC0); /* 47808 */
                        }
                        break;
                    case 'n':
                        result = atoi(optarg);
                        if (result) {
                            current->route_info.configuredNet.net =
                                (uint16_t)result;
                        }
                        else {
                            dbMessage(DBD_Config,
                                DB_ERROR,
                                "Error: Port %d remains unconfigured", current->port_support->datalinkId);
                            current->route_info.configuredNet.net = 0;        // unconfigured
                        }
                        break;
                    }
                    dev_opt =
                        getopt_long(argc,
                        argv,
                        bipString,
                        Options,
                        &index);
                }
                opt = dev_opt;
#endif
            }
            else
            {
                dbMessage(DBD_Config, DB_ERROR, "Error: %s unknown", optarg);
                return false;
            }
            break;
#endif

        case 'r':
            dbMessage_Enable(DBD_Application);
            break;

        case 't':
            bits_strlcpy(staticIfname, optarg, sizeof(staticIfname));
            bits_strlcpy(cmdIfname, staticIfname, sizeof(staticIfname));
            hardcodedTestConfig = true;
            dbMessage(DBD_Config, DB_NOTE, "Using hard-coded test configuration, on interface [%s]", cmdIfname );
            break;

        case '?':
        case ';':
            break;

        default:
            panic();
            break;
        }
    } while ( *(++optPtr) != 0 );

    return true;
}

