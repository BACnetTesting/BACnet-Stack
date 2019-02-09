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

#include "bacstr.h"
#include "osLayer.h"
#include "device.h"


BACNET_CHARACTER_STRING My_Object_Name;
uint32_t Object_Instance_Number;

#if defined ( __GNUC__ )
extern char firstEthernet[20];
#endif

bool read_config(char *filepath)
{
#if 0
    config_t cfg;
    config_setting_t *setting;
    // ROUTER_PORT *current = headRouterport;
    // ROUTER_PORT *current2;
    int result;      // , fd;
    static bool firstEthernetSet = false;
    const char *deviceName;
    int deviceInstance;
    int portId = 1;

    config_init(&cfg);

    /* open configuration file */
    if (!config_read_file(&cfg, filepath)) {
        PRINT(ERROR,
            "Config file error: %d - %s\n",
            config_error_line(&cfg),
            config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }

    if (config_lookup_string(&cfg, "device_name", &deviceName) == CONFIG_TRUE) {
        printf("Device Name exists - %s\n", deviceName);
        characterstring_init_ansi(&My_Object_Name, deviceName);
    }
    else {
        characterstring_init_ansi(&My_Object_Name, "Make this Device Name unique!");
    }

    if (config_lookup_int(&cfg, "device_instance", &deviceInstance) == CONFIG_TRUE) {
        printf("Device Instance exists - %d\n", deviceInstance);
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

            //            /* create new list node to store port information */
            //            if (headRouterport == NULL) {
            //                headRouterport = (ROUTER_PORT *)malloc(sizeof(ROUTER_PORT));
            //                if (headRouterport == NULL) {
            //                    return false;
            //                }
            //                bzero(headRouterport, sizeof(ROUTER_PORT));
            //                headRouterport->llist.next = NULL;
            //                current = headRouterport;
            //            }
            //            else {
            //                ROUTER_PORT *tmp = current;
            //                // current = (ROUTER_PORT *) current->llist.next;            // do not comment this out again without careful review
            //                current = (ROUTER_PORT *)malloc(sizeof(ROUTER_PORT));
            //                if (current == NULL) {
            //                    return false;
            //                }
            //                bzero(current, sizeof(ROUTER_PORT));
            //                current->llist.next = NULL;
            //                tmp->llist.next = (LLIST *) current;
            //            }

                        result = config_setting_lookup_string(port, "device", &iface);
            if (result == CONFIG_FALSE) continue ;
            // current->port_support->ifName = (char *)malloc((strlen(iface) + 1) * sizeof(char));

            if(!firstEthernetSet)
            {
                // this to send traffic stats to BACnet Traffic Analyzer
                firstEthernetSet = true;
                strcpy(firstEthernet, iface);
            }

            // strcpy ( (char *) current->port_support->ifName, iface);

            //                /* check if interface is valid */
            //                //                    printf("Interface1: %s\n", iface);
            //
            //                //                  fd = socket(AF_INET, SOCK_DGRAM, 0);
            //                //                  if (fd) {
            //                //                      struct ifreq ifr;
            //                //                      strncpy(ifr.ifr_name, current->iface,
            //                //                          sizeof(ifr.ifr_name) - 1);
            //                //                      result = ioctl(fd, SIOCGIFADDR, &ifr);
            //                //                      if (result != -1) {
            //                //                          close(fd);
            //                //                      } else {
            //                //                          PRINT(ERROR,
            //                //                              "Error: Invalid interface for BIP device 1\n");
            //                //                          return false;
            //                //                      }
            //                //                  }
            //            }
            //            else {
            //                current->port_support->ifName = "eth0";
            //            }

            result = config_setting_lookup_int(port, "port", &portNo);
            if (result == CONFIG_FALSE) continue ;
            //        {
            //                current->port_support->datalink.bipParams.nwoPort = htons( param );
            //            }
            //            else {
            //                current->port_support->datalink.bipParams.nwoPort = htons( 0xBAC0 ) ;
            //            }

            result = config_setting_lookup_int(port, "network", &networkNo);
            if (result == CONFIG_FALSE) continue ;
            //                current->route_info.configuredNet.net = param;
            //            }
            //            else {
            //                current->route_info.configuredNet.net = get_next_free_dnet();
            //            }

            // we no longer accept 'defaults', because a misconfigured router is so dangerous on an operating network we do not want to take any chances
            // and to that point we will not get here if any of the prior operations fail.

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
    Create_Device_Router(1, deviceInstance, deviceName, "Router", BACNET_VENDOR_ID, BACNET_VENDOR_NAME);

    config_destroy(&cfg);
    //     printf("cmd file parse success\r\n");

    AlignApplicationWithPort();
#endif
    return true;
}


bool parse_cmd(
    int argc,
    char *argv[])
{
#if 0
    const char *optString = "hsc:D:q";                // trailing : means option requires parameter, :: optional parameter
    const char *bipString = "p:n:D:";
    const struct option Options[] = {
        { "config", required_argument, NULL, 'c' },
        { "startDelay", required_argument, NULL, 's' },
        { "device", required_argument, NULL, 'D' },
        { "network", required_argument, NULL, 'n' },
        { "port", required_argument, NULL, 'P' },
        { "mac", required_argument, NULL, 'm' },
        { "quiet", required_argument, NULL, 'q' },
        { "help", no_argument, NULL, 'h' },
        { NULL, no_argument, NULL, 0 },
    };

    int opt, dev_opt, index, result;      // , fd;
    ROUTER_PORT *current = headRouterport;

    // if (argc < 2) print_help();

    /* begin checking cmd parameters */
    do
    {

        opt = getopt_long(argc, argv, optString, Options, &index);

        // -ve opt signals end of options..
        if(opt < 0)
        {
            printf("\n");
            break;
        }

        switch (opt) {
        case 's':
            printf("Start delay 30 seconds\n");
            sleep(30);
            break;

        case 'q':
            // 'quiet mode'
            // quiet_mode = true;
            break;

        case 'h':
            // print_help();
            return false;

        case 'c':
            printf("Reading config file %s\n", optarg);
            read_config(optarg);
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

                printf("Interface2: %s\n", current->port_support->ifName);

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
                            PRINT(ERROR, "Error: Port %d remains unconfigured\n", current->port_support->datalinkId);
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
                PRINT(ERROR, "Error: %s unknown\n", optarg);
                return false;
            }
            break;
        }
    } while (opt > 0);
#endif
    return true;
}
