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

#include <string.h>
#include <libconfig.h>      /* read config files, from libconfig */
#include <getopt.h>

#include "bitsRouter.h"
#include "bacstr.h"
#include "osLayer.h"
#include "device.h"
#include "bacversion.h"

ROUTER_PORT *headRouterport = NULL; /* pointer to list of router ports */
extern ROUTER_PORT *applicationRouterPort;

BACNET_CHARACTER_STRING My_Object_Name;
uint32_t Object_Instance_Number;

#if defined ( __GNUC__ )
extern char firstEthernet[20];
#endif

bool read_config(char *filepath)
{
    config_t cfg;
    config_setting_t *setting;
    // ROUTER_PORT *current = headRouterport;
    // ROUTER_PORT *current2;
    int result;      // , fd;
    static bool firstEthernetSet = false;
    const char *deviceName;
    int deviceInstance;
    int portId = 1;
    int appNetworkNumber = 0;

    config_init(&cfg);

    /* open configuration file */
    if (!config_read_file(&cfg, filepath)) {
        dbTraffic(DBD_ALL,
            DB_ERROR, 
            "Config file error: %d - %s\n",
            config_error_line(&cfg),
            config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }

    if (config_lookup_string(&cfg, "device_name", &deviceName) == CONFIG_TRUE) {
        dbTraffic(DBD_Config, DB_NOTE, "Device Name set to [%s]\n", deviceName);
        characterstring_init_ansi(&My_Object_Name, deviceName);
    }
    else {
        characterstring_init_ansi(&My_Object_Name, "Make this Device Name unique!");
    }

    if (config_lookup_int(&cfg, "device_instance", &deviceInstance) == CONFIG_TRUE) {
        dbTraffic(DBD_Config, DB_NOTE, "Device Instance set to [%d]\n", deviceInstance);
        Object_Instance_Number = deviceInstance;
    }

    /* get router "port" count */
    setting = config_lookup(&cfg, "ports");
    if (setting != NULL) {
        int count = config_setting_length(setting);
        int i;

        /* lookup and initialize router "port" parameters */
        for (i = 0; i < count; i++) {
            const char *iface;
            // long int param;
            int portNo;
            int networkNo;

            config_setting_t *port = config_setting_get_elem(setting, i);

            // we no longer accept 'defaults', because a misconfigured router is so dangerous on an operating network we do not want to take any chances
			// and to that point we will not get here if any of the prior operations fail.
			// hence the 'continue'.

			result = config_setting_lookup_string(port, "device", &iface);
            if (result == CONFIG_FALSE) continue ;

            result = config_setting_lookup_int(port, "port", &portNo);
            if (result == CONFIG_FALSE) continue ;

            result = config_setting_lookup_int(port, "network", &networkNo);
            if (result == CONFIG_FALSE) continue ;
            //                current->route_info.configuredNet.net = param;
            //            }
            //            else {
            //                current->route_info.configuredNet.net = get_next_free_dnet();
            //            }

            // we no longer accept 'defaults', because a misconfigured router is so dangerous on an operating network we do not want to take any chances
            // and to that point we will not get here if any of the prior operations fail.
            
//            if (!firstEthernetSet) {
//                // this to send traffic stats to BACnet Traffic Analyzer
//                firstEthernetSet = true;
//                strcpy(firstEthernet, iface);
//                appNetworkNumber = networkNo;
//            }

            InitRouterport(portId++, BPT_BIP, iface, networkNo, portNo);

    // now this gets intialized when the router entity is created (just below)
//            if (applicationRouterPort == NULL) {
//                if (!InitRouterportApp(networkNo)) {
//                    panic();
//                }
//            }
        }
    }
    else {
        config_destroy(&cfg);
        return false;
    }

    // Virtual_Router_Init(deviceInstance, deviceName, "Device Description");
    Create_Device_Router( 1, deviceInstance, deviceName, "Echelon BACnet/IP to BACnet/FT Router", BACNET_VENDOR_ID, BACNET_VENDOR_NAME);

    config_destroy(&cfg);
    
    AlignApplicationWithPort();

    return true;
}


char *configFileName ;
extern bool logToConsole;
extern bool sidestepSecurity;

bool parse_cmd(
    int argc,
    char *argv[])
{
    const char *optString = "ifhsc:D:qve:g:";                // trailing : means option requires parameter, :: optional parameter
    const char *bipString = "p:n:D:";
    const struct option Options[] = {
        { "config", required_argument, NULL, 'c' },
//        { "startDelay", required_argument, NULL, 's' },
        { "device", required_argument, NULL, 'D' },
        { "network", required_argument, NULL, 'n' },
        { "verbosityLevel", required_argument, NULL, 'e' },
        { "port", required_argument, NULL, 'P' },
        { "mac", required_argument, NULL, 'm' },
        { "quiet", required_argument, NULL, 'q' },
        { "help", no_argument, NULL, 'h' },
        { "interactive_mode", no_argument, NULL, 'i' },
        { "security", no_argument, NULL, 's' },
        { "ignore", required_argument, NULL, 'g' },
        { NULL, no_argument, NULL, 0 },
    };

    int opt, dev_opt, index, result;      // , fd;
    int dblev;
    ROUTER_PORT *current = headRouterport;

    /* begin checking cmd parameters */
    do
    {

        opt = getopt_long(argc, argv, optString, Options, &index);

        // -ve opt signals end of options..
        if(opt < 0)
        {
            break;
        }

        switch (opt) {
        case 's':
            sidestepSecurity = true;
            break;

        case 'i':
            logToConsole = true;
            break;
            
        case 'q':
            // 'quiet mode'
            // quiet_mode = true;
            break;

        case 'h':
            // print_help();
            logToConsole = true;
            ShowCommandlineHelp();
            exit(-1);
            break;
            
        case 'e':
        {
            dblev = atoi(optarg);
            dbTrafficSetLevel(dblev);
            int oldConsole = logToConsole;
            logToConsole = true;
            dbTraffic(DBD_Config, DB_NOTE, "Debug level set to %d", dblev);
            logToConsole = oldConsole;
        }
            break;

        case 'g':
            // ignore trailing parameter
            break;

        case 'v':
            logToConsole = true;
            dbTraffic(DBD_Config, DB_ALWAYS, "%s", "Version: " BACNET_VERSION_TEXT);
            exit(0);
            break;

        case 'c':
            // we will report this when we actually read the file printf("Reading config file %s\n", optarg);
            configFileName = optarg;
            // read_config(optarg);
            break;

        case 'D':

            /* create new list node to store port information */
            if (headRouterport == NULL) {
                headRouterport = (ROUTER_PORT *)malloc(sizeof(ROUTER_PORT));
                if (headRouterport == NULL) return false;
                headRouterport->llist.next = NULL;
                current = headRouterport;
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

                dbTraffic(DBD_Config, DB_NOTE, "Interface2: %s\n", current->port_support->ifName);

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
                //                              "Error: Invalid interface for BIP device 2\n");
                //                          return false;
                //                      }
                //                  }

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
                            dbTraffic(DBD_Config,
                                DB_ERROR,
                                "Error: Port %d remains unconfigured\n", current->port_support->datalinkId);
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
            }
            else
            {
                dbTraffic(DBD_Config, DB_ERROR, "Error: %s unknown\n", optarg);
                return false;
            }
            break;
        }
    } while (opt > 0);

    return true;
}
