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

#include "bitsRouter.h"
#include "bacstr.h"
#include "osLayer.h"
#include "device.h"
#include "bacversion.h"
#include "appApi.h"
#include "bitsUtil.h"

extern ROUTER_PORT* headRouterPort;
ROUTER_PORT *headRouterport = NULL; /* pointer to list of router ports */
extern ROUTER_PORT *applicationRouterPort;

// "root name" for virtual devices, and "base instance" for virtual device instances TBD, needs UI changes first, tracked using
uint32_t deviceInstanceBase = 370000 ;    // this is just a default...
int data_age_limit = 4;

BACNET_CHARACTER_STRING My_Object_Name;
// uint32_t Object_Instance_Number;
extern unsigned simulationCount;

// extern bool hardcodedTestConfig;

//#if defined ( __GNUC__ )
//extern char firstEthernet[20];
//#endif

static void process_config_common(config_t *cfg)
{
    int virtualDeviceInstance;

    if (config_lookup_int( cfg, "virtual_device_base_instance", &virtualDeviceInstance) == CONFIG_TRUE) {
        deviceInstanceBase = (uint32_t) virtualDeviceInstance;
        dbMessage(DBD_Config, DB_NOTE, "Virtual Device Base Instance set to [%u]", deviceInstanceBase);
    }
}

void Create_Objects(const uint devInstance, const BACNET_OBJECT_TYPE objType, const char* name, const uint count )
{
    for (uint objInstance = 1; objInstance <= count; objInstance++)
    {
        char objectName[100];
        char objectDesc[100];
        sprintf(objectName, "%s:%u", name, objInstance);
        sprintf(objectDesc, "Description for %s %u", name, objInstance);
        Create_Object(devInstance, objType, objInstance, objectName, objectDesc );
    }
}



bool read_config(const char *filepath)
{
    config_t cfg;
    config_setting_t *setting;
    int result;
    static bool firstEthernetSet = false;
    const char *deviceName = "mxxxx - Not set by config" ;
    int routerDeviceInstance = 100099 ;
    int portId = 1;
    int appNetworkNumber = 0;
    int vNN = 1089 ;    // todo 1 - change this back to 0 once virtual_network_number field installed.
    // todo 3 - also sanity check for duplicates on startup.

    config_init(&cfg);

    /* open configuration file */
    if (!config_read_file(&cfg, filepath)) {
        dbMessage(DBD_ALL,
            DB_ERROR,
            "Config file error: %d - %s",
            config_error_line(&cfg),
            config_error_text(&cfg));
        config_destroy(&cfg);
        return false;
    }

    process_config_common(&cfg);

    if (config_lookup_string(&cfg, "device_name", &deviceName) == CONFIG_TRUE) {
        dbMessage(DBD_Config, DB_NOTE, "Device Name set to [%s]", deviceName);
        characterstring_init_ansi(&My_Object_Name, deviceName);
    }
    else {
        characterstring_init_ansi(&My_Object_Name, "Make this Device Name unique!");
    }

    if (config_lookup_int(&cfg, "device_instance", &routerDeviceInstance) == CONFIG_TRUE) {
        dbMessage(DBD_Config, DB_NOTE, "Device Instance set to [%d]", routerDeviceInstance);
        // Object_Instance_Number = routerDeviceInstance;
    }

    if (config_lookup_int(&cfg, "virtual_network_number", &vNN) == CONFIG_TRUE) {
        dbMessage(DBD_Config, DB_NOTE, "Virtual_Network_Number set to [%d]", vNN);
    }

    if (config_lookup_int(&cfg, "data_age_limit", &data_age_limit) == CONFIG_TRUE) {
        dbMessage(DBD_Config, DB_NOTE, "data_age_limit set to [%d]", data_age_limit);
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
            int isolated = 0;

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

            result = config_setting_lookup_bool(port, "isolated",&isolated);
            if (isolated) {
                if (isolatedRouterPort != NULL) {
                    dbMessage(DBD_Config, DB_NOTE, "Isolated interface already defined; %s will not be isolated.", iface);
                    isolated = false;
                } else {
                    dbMessage(DBD_Config, DB_NOTE, "interface %s is isolated.", iface);
                }
            }
            // Don't check result. 'isolated' key may not be present, no reason to punt, default is false.
            
            if(strstr(iface, "lon") != NULL) {
                // todo 3 - revert to portNo when BAClon can accommodate different BACnet IP port numbers
                InitRouterport(portId++, BPT_FT, iface, networkNo, 47808, isolated);
            }
            else {
                if (isolated) {
                    InitRouterport(portId++, BPT_BIP, iface, networkNo, portNo, isolated);
                } else {
                    InitRouterport(portId++, BPT_BBMD, iface, networkNo, portNo, isolated);
                }
            }
        }
    }
    else {
        config_destroy(&cfg);
        return false;
    }

    Create_Device_Router( headRouterPort->port_id, routerDeviceInstance, deviceName, "BACnet/IP to BACnet/FT Router", BACNET_VENDOR_ID, BACNET_VENDOR_NAME);

    config_destroy(&cfg);

    AlignApplicationWithPort();

    // we no longer accept 'defaults', because a misconfigured router is so dangerous on an operating network we do not want to take any chances
    // and to that point we will not get here if any of the prior operations fail.
    if (vNN != 0) {
        InitRouterportVirtual(PORTID_VIRT, vNN);
    }

    if (simulationCount)
    {
        for (uint i = 0; i < simulationCount; i++)
        {
            char deviceName[100];
            char objectName[100];
            uint instance = deviceInstanceBase + i;

            sprintf(deviceName, "Virtual device %u", instance);

            Create_Device_Virtual(PORTID_VIRT, instance, deviceName, "Virtual Device mapping to IAP device", BACNET_VENDOR_ID, BACNET_VENDOR_NAME);     // todo 2, add model?

            Create_Objects(instance, OBJECT_ANALOG_INPUT , "Analog Input", 4);
            Create_Objects(instance, OBJECT_ANALOG_OUTPUT, "Analog Output", 4);
        }
    }

    /*
 
     * - how to mitigate CPU -
     * - stop using tables, use lists
     * - stop using llists - use maps.... (searching for devices/objects is time consuming...
     *
     *here is one map: https://github.com/rxi/map
     *and another that I have used https://github.com/troydhanson/uthash
     *
     * - how to mitigate mem
     * - there are all sorts of structure variables / tables for devices / objects.
     *  - use llists or maps for these
     *  - bacnet strings use fixes space, make dynamically sized strings...
     *
     **/

    return true;
}


void read_config_auxiliary(const char *filepath)
{
    config_t cfg;

    config_init(&cfg);

    /* open configuration file */
    if (!config_read_file(&cfg, filepath)) {
        dbMessage(DBD_Config, DB_NOTE, "Auxiliary Config File [%s] does not exist. Ignoring.", filepath );
        config_destroy(&cfg);
        return ;
    }
    dbMessage(DBD_Config, DB_NOTE, "Auxiliary Config File [%s] processed OK.", filepath);

    process_config_common(&cfg);

    config_destroy(&cfg);
}



