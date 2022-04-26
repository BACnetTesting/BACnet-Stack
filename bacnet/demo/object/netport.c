/**
 * @file
 * @author Steve Karg
 * @date 2016
 * @brief Network port objects, customize for your use
 *
 * @section DESCRIPTION
 *
 * The Network Port object provides access to the configuration
 * and properties of network ports of a device.
 *
 * @section LICENSE
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************************
*
*   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*   For more information: info@bac-test.com
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
*  2018.07.04 EKH Diffed in hints from Binary Value for future reference
*
****************************************************************************************/

#include <string.h>
#include "configProj.h"     /* the custom stuff */

#if ( BACNET_PROTOCOL_REVISION >= 17 )

#include "handlers.h"
#include "netport.h"
#include "bitsDebug.h"
#include "llist.h"
#include "emm.h"
// #include "bacnetObject.h"

static LLIST_HDR NP_Descriptor_List;

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_OUT_OF_SERVICE,
    PROP_RELIABILITY,
    PROP_STATUS_FLAGS,
    PROP_NETWORK_TYPE,
    PROP_PROTOCOL_LEVEL,
    PROP_NETWORK_NUMBER,
    PROP_NETWORK_NUMBER_QUALITY,
    PROP_CHANGES_PENDING,
    PROP_APDU_LENGTH,
    PROP_LINK_SPEED,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Optional[] = {
    PROP_MAC_ADDRESS,
    PROP_MAX_APDU_LENGTH_ACCEPTED,
    
    // todo - these may have to become dynamic for diff network port objects!
#if defined(BACDL_MSTP)

    PROP_MAX_MASTER,
    PROP_MAX_INFO_FRAMES,

#elif defined(BACDL_BIP)

    PROP_BACNET_IP_MODE,
    PROP_IP_ADDRESS,
    PROP_BACNET_IP_UDP_PORT,
    PROP_IP_SUBNET_MASK,
    PROP_IP_DEFAULT_GATEWAY,
    PROP_IP_DNS_SERVER,

#if defined(BBMD_ENABLED)
    PROP_BBMD_ACCEPT_FD_REGISTRATIONS,
    PROP_BBMD_BROADCAST_DISTRIBUTION_TABLE,
    PROP_BBMD_FOREIGN_DEVICE_TABLE,
#endif

#endif

    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


void Network_Port_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    // Do NOT be tempted to use property_list_required() - that is for supporting epics.c, and perhaps other Client operations, only
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


// Gets called once for each device
// This only gets called once on startup, and has to initialize for ALL virtual devices. Todo 2 - rename to differentiate!
void Network_Port_Init(
    void)
{
    ll_Init(&NP_Descriptor_List, 100);
}


bool Network_Port_Create(
    const uint32_t instance,
    const char *name)
{
    NETWORK_PORT_DESCR *currentObject = (NETWORK_PORT_DESCR *)emm_scalloc('a', sizeof(NETWORK_PORT_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&NP_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    // Note that our structure is 0 initialized by calloc, so no zeroing operations are _required_.
    // Some are here just for clarity.
    currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;

    Init_Datalink_Thread();

    return true;
}


bool Network_Port_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&NP_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Network_Port_Count(
    void)
{
    return NP_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
uint32_t Network_Port_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&NP_Descriptor_List, index);
}


//unsigned Network_Port_Instance_To_Index(
//    uint32_t object_instance)
//{
//    unsigned index = 0;
//
//    for (index = 0; index < BACNET_NETWORK_PORTS_MAX; index++) {
//        if (currentObject->Instance_Number == object_instance) {
//            return index;
//        }
//    }
//
//    return BACNET_NETWORK_PORTS_MAX;
//}

/**
 * For the Network Port object, set the instance number.
 *
 * @param  index - 0..BACNET_NETWORK_PORTS_MAX value
 * @param  object_instance - object-instance number of the object
 *
 * @return  true if values are within range and property is set.
 */
void Network_Port_Set_Network_Port_Instance_ID(
    NETWORK_PORT_DESCR *currentObject,
    uint32_t object_instance)
{
    currentObject->common.objectInstance = object_instance;
}


//bool Network_Port_Out_Of_Service(
//    uint32_t object_instance)
//{
//    bool oos_flag = false;
//    unsigned index = 0;
//
//    index = Network_Port_Instance_To_Index(object_instance);
//    if (index < BACNET_NETWORK_PORTS_MAX) {
//        oos_flag = currentObject->Out_Of_Service;
//    }
//
//    return oos_flag;
//}
//
//
//bool Network_Port_Out_Of_Service_Set(
//    uint32_t object_instance,
//    bool value)
//{
//    bool status = false;
//    unsigned index = 0;
//
//    index = Network_Port_Instance_To_Index(object_instance);
//    if (index < BACNET_NETWORK_PORTS_MAX) {
//        currentObject->Out_Of_Service = value;
//        status = true;
//    }
//
//    return status;
//}

static inline bool isOutOfService(NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Out_Of_Service;
}


static inline bool isInAlarm(NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Event_State != EVENT_STATE_NORMAL;
    return false;
}


BACNET_RELIABILITY Network_Port_Reliability_Get(
    NETWORK_PORT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadowReliability ;
    }

    // In this reference stack, nobody ever actually sets reliability, we expect the Application to do so (along with PV).
    return currentObject->Reliability;
}


static bool isInFault(
    NETWORK_PORT_DESCR *currentObject)
{
    return (Network_Port_Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


NETWORK_PORT_DESCR *Network_Port_Instance_To_Object(
    uint32_t object_instance)
{
    return (NETWORK_PORT_DESCR *)Generic_Instance_To_Object(&NP_Descriptor_List, object_instance);
}


void Network_Port_Reliability_Set(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_RELIABILITY value)
{
    currentObject->Reliability = value;
}


uint8_t Network_Port_Type(
    NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Network_Type;
}


void Network_Port_Type_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value)
{
    currentObject->Network_Type = value;
}

/**
 * For a given object instance-number, gets the BACnet Network Number.
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return BACnet network type value
 */
uint16_t Network_Port_Network_Number(
    NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Network_Number;
}


void Network_Port_Network_Number_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint16_t value)
{
    currentObject->Network_Number = value;
}


BACNET_PORT_QUALITY Network_Port_Quality(
    NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Quality;
}


void Network_Port_Quality_Set(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_PORT_QUALITY value)
{
    currentObject->Quality = value;
}

/**
 * For a given object instance-number, loads the mac-address into
 * an octet string.
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  mac_address - holds the mac-address retrieved
 *
 * @return  true if mac-address was retrieved
 */
bool Network_Port_MAC_Address(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *mac_address)
{
    bool status = false;
    uint8_t *mac = NULL;
    uint8_t ip_mac[4+2] = {0};
    size_t mac_len = 0;

        switch ( currentObject->Network_Type) {
            case PORT_TYPE_ETHERNET:
                mac = &currentObject->Network.Ethernet.MAC_Address[0];
                mac_len = sizeof(currentObject->Network.Ethernet.MAC_Address);
                break;
            case PORT_TYPE_MSTP:
                mac = &currentObject->Network.MSTP.MAC_Address;
                mac_len = sizeof(currentObject->Network.MSTP.MAC_Address);
                break;
            case PORT_TYPE_BIP:
                memcpy(&ip_mac[0],
                    &currentObject->Network.IPv4.IP_Address, 4);
                memcpy(&ip_mac[4],
                    &currentObject->Network.IPv4.Port, 2);
                mac = &ip_mac[0];
                mac_len = sizeof(ip_mac);
                break;
            case PORT_TYPE_BIP6:
                mac = &currentObject->Network.IPv6.MAC_Address[0];
                mac_len = sizeof(currentObject->Network.IPv6.MAC_Address);
                break;
            default:
                panic();
                return false;
        }
        if (mac) {
            status = octetstring_init(mac_address, mac, mac_len);
        }
        else {
            panic();
        }


    return status ;
}

/**
 * For a given object instance-number, sets the mac-address and it's length
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  new_name - holds the object-name to be written
 *         Expecting a pointer to a static ANSI C string for zero copy.
 *
 * @return  true if object-name was set
 */
void Network_Port_MAC_Address_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t *mac_src,
    uint8_t mac_len)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;
    size_t mac_size = 0;
    uint8_t *mac_dest = NULL;

        switch (currentObject->Network_Type) {
            case PORT_TYPE_ETHERNET:
                mac_dest = &currentObject->Network.Ethernet.MAC_Address[0];
                mac_size = sizeof(currentObject->Network.Ethernet.MAC_Address);
                break;
            case PORT_TYPE_MSTP:
                mac_dest = &currentObject->Network.MSTP.MAC_Address;
                mac_size = sizeof(currentObject->Network.MSTP.MAC_Address);
                break;
            case PORT_TYPE_BIP:
                /* no need to set - created from IP address and UPD Port */
                break;
            case PORT_TYPE_BIP6:
                mac_dest = &currentObject->Network.IPv6.MAC_Address[0];
                mac_size = sizeof(currentObject->Network.IPv6.MAC_Address);
                break;
            default:
                break;
        }
        if (mac_src && mac_dest && (mac_len == mac_size)) {
            memcpy(mac_dest, mac_src, mac_size);
            status = true;
        }
}

/**
 * For a given object instance-number, gets the BACnet Network Number.
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return APDU length for this network port
 */
uint16_t Network_Port_APDU_Length(
    NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->APDU_Length;
}

/**
 * For a given object instance-number, sets the BACnet Network Number
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - APDU length 0..65535
 *
 * @return  true if values are within range and property is set.
 */
void Network_Port_APDU_Length_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint16_t value)
{
    currentObject->APDU_Length = value;
}


    //uint32_t Network_Port_Link_Speed(
    //    uint32_t object_instance);
    //bool Network_Port_Link_Speed_Set(
    //    uint32_t object_instance,
    //    uint32_t value);

/**
 * For a given object instance-number, gets the network communication rate
 * as the number of bits per second. A value of 0 indicates an unknown
 * communication rate.
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return Link_Speed for this network port, or 0 if unknown
 */
uint32_t Network_Port_Link_Speed(
    NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Link_Speed;
}


/**
 * For a given object instance-number, sets the Link_Speed
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - APDU length 0..65535
 *
 * @return  true if values are within range and property is set.
 */
void Network_Port_Link_Speed_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint32_t value)
{
    currentObject->Link_Speed = value;
}


bool Network_Port_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING *object_name)
{
    return Generic_Instance_To_Object_Name(
        &NP_Descriptor_List,
        object_instance, object_name);
}


bool Network_Port_Name_Set(
    NETWORK_PORT_DESCR *currentObject,
    char *new_name)
{
    return characterstring_init_ansi( &currentObject->common.objectName, new_name );
}


/**
 * For a given object instance-number, returns the changes-pending
 * property value
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  changes-pending property value
 */
bool Network_Port_Changes_Pending(
    NETWORK_PORT_DESCR *currentObject)
{
    return currentObject->Changes_Pending;
}


/**
 * For a given object instance-number, sets the changes-pending property value
 *
 * @param object_instance - object-instance number of the object
 * @param value - boolean changes-pending value
 *
 * @return true if the changes-pending property value was set
 */
void Network_Port_Changes_Pending_Set(
    NETWORK_PORT_DESCR *currentObject,
    bool value)
{
    currentObject->Changes_Pending = value;
}


/**
 * For a given object instance-number, gets the MS/TP Max_Master value
 * Note: depends on Network_Type being set to PORT_TYPE_MSTP for this object
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return MS/TP Max_Master value
 */
uint8_t Network_Port_MSTP_Max_Master(
    NETWORK_PORT_DESCR *currentObject)
{
        if (currentObject->Network_Type == PORT_TYPE_MSTP) {
            return currentObject->Network.MSTP.Max_Master;
        }
        return 0;
}

/**
 * For a given object instance-number, sets the MS/TP Max_Master value
 * Note: depends on Network_Type being set to PORT_TYPE_MSTP for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - MS/TP Max_Master value 0..127
 *
 * @return  true if values are within range and property is set.
 */
bool Network_Port_MSTP_Max_Master_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value)
{
    bool status = false;
        if (currentObject->Network_Type == PORT_TYPE_MSTP) {
            if (value <= 127) {
                if (currentObject->Network.MSTP.Max_Master != value) {
                    currentObject->Changes_Pending = true;
                }
                currentObject->Network.MSTP.Max_Master = value;
                status = true;
            }
        }
        return status;
}


/**
 * For a given object instance-number, loads the ip-address into
 * an octet string.
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  ip_address - holds the mac-address retrieved
 *
 * @return  true if ip-address was retrieved
 */
bool Network_Port_IP_Address(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *ip_address)
{
        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            return octetstring_init(ip_address,
                &currentObject->Network.IPv4.IP_Address[0], 4);
        }
        return false;
}


/**
 * For a given object instance-number, sets the ip-address
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  a - ip-address first octet
 * @param  b - ip-address first octet
 * @param  c - ip-address first octet
 * @param  d - ip-address first octet
 *
 * @return  true if ip-address was set
 */
void Network_Port_IP_Address_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    if (currentObject->Network_Type == PORT_TYPE_BIP) {
        currentObject->Network.IPv4.IP_Address[0] = a;
        currentObject->Network.IPv4.IP_Address[1] = b;
        currentObject->Network.IPv4.IP_Address[2] = c;
        currentObject->Network.IPv4.IP_Address[3] = d;
    }
}

/**
 * For a given object instance-number, loads the subnet-mask-address into
 * an octet string.
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  subnet-mask - holds the mac-address retrieved
 *
 * @return  true if ip-address was retrieved
 */
bool Network_Port_IP_Subnet(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *subnet_mask)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;
    uint32_t mask = 0;
    uint32_t prefix = 0;
    uint8_t ip_mask[4] = {0};

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            prefix = currentObject->Network.IPv4.IP_Subnet_Prefix;
            mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
            encode_unsigned32(ip_mask, mask);
            status = octetstring_init(subnet_mask, ip_mask, sizeof(ip_mask));
        }

    return status;
}

/**
 * For a given object instance-number, gets the BACnet/IP Subnet prefix value
 * Note: depends on Network_Type being set to PORT_TYPE_BIP for this object
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return BACnet/IP subnet prefix value
 */
uint8_t Network_Port_IP_Subnet_Prefix(
    NETWORK_PORT_DESCR *currentObject)
{
    uint8_t value = 0;
    unsigned index = 0;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            value = currentObject->Network.IPv4.IP_Subnet_Prefix;
        }
    return value;
}

/**
 * For a given object instance-number, sets the BACnet/IP Subnet prefix value
 * Note: depends on Network_Type being set to PORT_TYPE_BIP for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - BACnet/IP Subnet prefix value 1..32
 *
 * @return  true if values are within range and property is set.
 */
void Network_Port_IP_Subnet_Prefix_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value)
{
    unsigned index = 0;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            if (value <= 32) {
                if (currentObject->Network.IPv4.IP_Subnet_Prefix != value) {
                    currentObject->Changes_Pending = true;
                }
                currentObject->Network.IPv4.IP_Subnet_Prefix = value;
            }
        }
}

/**
 * For a given object instance-number, loads the gateway ip-address into
 * an octet string.
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  ip_address - holds the ip-address retrieved
 *
 * @return  true if ip-address was retrieved
 */
bool Network_Port_IP_Gateway(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *ip_address)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            status = octetstring_init(ip_address,
                &currentObject->Network.IPv4.IP_Gateway[0], 4);
        }

    return status;
}

/**
 * For a given object instance-number, sets the gateway ip-address
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  a - ip-address first octet
 * @param  b - ip-address first octet
 * @param  c - ip-address first octet
 * @param  d - ip-address first octet
 *
 * @return  true if ip-address was set
 */
void Network_Port_IP_Gateway_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    unsigned index = 0; /* offset from instance lookup */

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            currentObject->Network.IPv4.IP_Gateway[0] = a;
            currentObject->Network.IPv4.IP_Gateway[1] = b;
            currentObject->Network.IPv4.IP_Gateway[2] = c;
            currentObject->Network.IPv4.IP_Gateway[3] = d;
        }
}

/**
 * For a given object instance-number, loads the subnet-mask-address into
 * an octet string.
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  dns_index - 0=primary, 1=secondary, 3=tertierary
 * @param  ip_address - holds the mac-address retrieved
 *
 * @return  true if ip-address was retrieved
 */
bool Network_Port_IP_DNS_Server(
    NETWORK_PORT_DESCR *currentObject,
    unsigned dns_index,
    BACNET_OCTET_STRING *ip_address)
{
    bool status = false;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            if (dns_index < BIP_DNS_MAX) {
                status = octetstring_init(ip_address,
                    &currentObject->Network.IPv4.IP_DNS_Server[dns_index][0],
                    4);
            }
        }

    return status;
}

/**
 * For a given object instance-number, sets the ip-address
 * Note: depends on Network_Type being set for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  index - 0=primary, 1=secondary, 3=tertierary
 * @param  a - ip-address first octet
 * @param  b - ip-address first octet
 * @param  c - ip-address first octet
 * @param  d - ip-address first octet
 *
 * @return  true if ip-address was set
 */
void Network_Port_IP_DNS_Server_Set(
    NETWORK_PORT_DESCR *currentObject,
    unsigned dns_index,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            if (dns_index < BIP_DNS_MAX) {
                currentObject->Network.IPv4.IP_DNS_Server[dns_index][0] = a;
                currentObject->Network.IPv4.IP_DNS_Server[dns_index][1] = b;
                currentObject->Network.IPv4.IP_DNS_Server[dns_index][2] = c;
                currentObject->Network.IPv4.IP_DNS_Server[dns_index][3] = d;
            }
        }

}

/**
 * For a given object instance-number, gets the BACnet/IP UDP Port number
 * Note: depends on Network_Type being set to PORT_TYPE_BIP for this object
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return BACnet/IP UDP Port number
 */
uint16_t Network_Port_BIP_Port(
    NETWORK_PORT_DESCR *currentObject)
{
    uint16_t value = 0;
        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            value = currentObject->Network.IPv4.Port;
        }

    return value;
}

/**
 * For a given object instance-number, sets the BACnet/IP UDP Port number
 * Note: depends on Network_Type being set to PORT_TYPE_BIP for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - BACnet/IP UDP Port number (default=0xBAC0)
 *
 * @return  true if values are within range and property is set.
 */
void Network_Port_BIP_Port_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint16_t value)
{
    unsigned index = 0;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            if (value <= 32) {
                if (currentObject->Network.IPv4.Port != value) {
                    currentObject->Changes_Pending = true;
                }
                currentObject->Network.IPv4.Port = value;
            }
        }
}

/**
 * For a given object instance-number, gets the BACnet/IP UDP Port number
 * Note: depends on Network_Type being set to PORT_TYPE_BIP for this object
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return BACnet/IP UDP Port number
 */
BACNET_IP_MODE Network_Port_BIP_Mode(
    NETWORK_PORT_DESCR *currentObject)
{
    BACNET_IP_MODE value ;

        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            value = (BACNET_IP_MODE) currentObject->Network.IPv4.Mode;
        }

    return value;
}

/**
 * For a given object instance-number, sets the BACnet/IP UDP Port number
 * Note: depends on Network_Type being set to PORT_TYPE_BIP for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - BACnet/IP UDP Port number (default=0xBAC0)
 *
 * @return  true if values are within range and property is set.
 */
bool Network_Port_BIP_Mode_Set(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_IP_MODE value)
{
    bool status = false;
        if (currentObject->Network_Type == PORT_TYPE_BIP) {
            if (value <= 32) {
                if (currentObject->Network.IPv4.Mode != value) {
                    currentObject->Changes_Pending = true;
                }
                currentObject->Network.IPv4.Mode = value;
                status = true;
            }
        }

    return status;
}

/**
 * For a given object instance-number, returns the BBMD-Accept-FD-Registrations
 * property value
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  BBMD-Accept-FD-Registrations property value
 */
bool Network_Port_BBMD_Accept_FD_Registrations(
    NETWORK_PORT_DESCR *currentObject)
{
    bool flag = false;
    struct bacnet_ipv4_port *ipv4 = NULL;

        ipv4 = &currentObject->Network.IPv4;
        flag = ipv4->BBMD_Accept_FD_Registrations;

    return flag;
}

/**
 * For a given object instance-number, sets the BBMD-Accept-FD-Registrations
 * property value
 *
 * @param object_instance - object-instance number of the object
 * @param flag - boolean changes-pending flag
 *
 * @return true if the BBMD-Accept-FD-Registrations property value was set
 */
bool Network_Port_BBMD_Accept_FD_Registrations_Set(
    NETWORK_PORT_DESCR *currentObject,
    bool flag)
{
    bool status = false;
    struct bacnet_ipv4_port *ipv4 = NULL;

        ipv4 = &currentObject->Network.IPv4;
        if (flag != ipv4->BBMD_Accept_FD_Registrations) {
            ipv4->BBMD_Accept_FD_Registrations = flag;
            currentObject->Changes_Pending = true;
        }
        status = true;

    return status;
}

/**
 * For a given object instance-number, gets the MS/TP Max_Info_Frames value
 * Note: depends on Network_Type being set to PORT_TYPE_MSTP for this object
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return MS/TP Max_Info_Frames value
 */
uint8_t Network_Port_MSTP_Max_Info_Frames(
    NETWORK_PORT_DESCR *currentObject)
{
    uint8_t value = 0;
        if (currentObject->Network_Type == PORT_TYPE_MSTP) {
            value = currentObject->Network.MSTP.Max_Info_Frames;
        }

    return value;
}

/**
 * For a given object instance-number, sets the MS/TP Max_Info_Frames value
 * Note: depends on Network_Type being set to PORT_TYPE_MSTP for this object
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - MS/TP Max_Info_Frames value 0..255
 *
 * @return  true if values are within range and property is set.
 */
bool Network_Port_MSTP_Max_Info_Frames_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value)
{

        if (currentObject->Network_Type == PORT_TYPE_MSTP) {
            if (currentObject->Network.MSTP.Max_Info_Frames != value) {
                currentObject->Changes_Pending = true;
            }
            currentObject->Network.MSTP.Max_Info_Frames = value;
            return true;
        }

        return false;
}


static void Network_Port_Out_Of_Service_Set(
    NETWORK_PORT_DESCR *currentObject,
    const bool oos_flag)
{
    currentObject->Out_Of_Service = oos_flag;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
int Network_Port_Read_Property(
    BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;   /* return value */
//    BACNET_BIT_STRING bit_string;
    BACNET_OCTET_STRING octet_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu;

    if ((rpdata == NULL) ||
        (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }

    NETWORK_PORT_DESCR *currentObject = Network_Port_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        // the following line does not appear in other object types. Should we do so todo 1
        rpdata->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    apdu = rpdata->application_data;

    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_NETWORK_PORT,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
        Network_Port_Object_Name(
            rpdata->object_instance,
            &char_string);
        apdu_len = encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len = encode_application_enumerated(&apdu[0], OBJECT_NETWORK_PORT);
        break;

    case PROP_STATUS_FLAGS:
        apdu_len =
            encode_status_flags(
                apdu,
                isInAlarm(currentObject),
                isInFault(currentObject),
                isOutOfService(currentObject));
        break;

    case PROP_RELIABILITY:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                Network_Port_Reliability_Get(currentObject));
        break;

    case PROP_OUT_OF_SERVICE:
        apdu_len =
            encode_application_boolean(
                apdu,
                isOutOfService(currentObject));
        break;

    case PROP_NETWORK_TYPE:
        apdu_len = encode_application_enumerated(&apdu[0],
            Network_Port_Type(currentObject));
        break;

    case PROP_PROTOCOL_LEVEL:
        apdu_len = encode_application_enumerated(&apdu[0],
            BACNET_PROTOCOL_LEVEL_BACNET_APPLICATION);
        break;

    case PROP_NETWORK_NUMBER:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_Network_Number(currentObject));
        break;

    case PROP_NETWORK_NUMBER_QUALITY:
        apdu_len = encode_application_enumerated(&apdu[0],
            Network_Port_Quality(currentObject));
        break;

    case PROP_MAC_ADDRESS:
        Network_Port_MAC_Address(currentObject, &octet_string);
        apdu_len = encode_application_octet_string(&apdu[0], &octet_string);
        break;

    case PROP_MAX_APDU_LENGTH_ACCEPTED:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_APDU_Length(currentObject));
        break;

    case PROP_LINK_SPEED:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_Link_Speed(currentObject));
        break;

    case PROP_CHANGES_PENDING:
        apdu_len = encode_application_boolean(&apdu[0],
            Network_Port_Changes_Pending(currentObject));
        break;

    case PROP_APDU_LENGTH:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_APDU_Length(currentObject));
        break;

#if defined(BACDL_MSTP)
    case PROP_MAX_MASTER:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_MSTP_Max_Master(rpdata->object_instance));
        break;
    case PROP_MAX_INFO_FRAMES:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_MSTP_Max_Info_Frames(rpdata->object_instance));
        break;
#endif

#if defined(BACDL_BIP)
    case PROP_BACNET_IP_MODE:
        apdu_len = encode_application_enumerated(&apdu[0],
            Network_Port_BIP_Mode(currentObject));
        break;
    case PROP_IP_ADDRESS:
        Network_Port_IP_Address(currentObject, &octet_string);
        apdu_len = encode_application_octet_string(&apdu[0], &octet_string);
        break;
    case PROP_BACNET_IP_UDP_PORT:
        apdu_len = encode_application_unsigned(&apdu[0],
            Network_Port_BIP_Port(currentObject));
        break;
    case PROP_IP_SUBNET_MASK:
        Network_Port_IP_Subnet(currentObject, &octet_string);
        apdu_len = encode_application_octet_string(&apdu[0], &octet_string);
        break;
    case PROP_IP_DEFAULT_GATEWAY:
        Network_Port_IP_Gateway(currentObject, &octet_string);
        apdu_len = encode_application_octet_string(&apdu[0], &octet_string);
        break;
     case PROP_IP_DNS_SERVER:
        if (rpdata->array_index == 0)
            /* Array element zero is the number of objects in the list */
            apdu_len = encode_application_unsigned(&apdu[0], BIP_DNS_MAX);
        else if (rpdata->array_index == BACNET_ARRAY_ALL) {
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            int len;
            for (unsigned i = 0; i < BIP_DNS_MAX; i++) {
                Network_Port_IP_DNS_Server(currentObject, i,
                    &octet_string);
                len = encode_application_octet_string(&apdu[apdu_len],
                    &octet_string);
                apdu_len += len;
            }
        } else if (rpdata->array_index <= BIP_DNS_MAX) {
            /* index was specified; encode a single array element */
            unsigned index;
            index = rpdata->array_index - 1;
            Network_Port_IP_DNS_Server(currentObject, index,
                &octet_string);
            apdu_len = encode_application_octet_string(&apdu[0],
                &octet_string);
        } else {
            /* index was specified, but out of range */
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
            apdu_len = BACNET_STATUS_ERROR;
        }
        break;

#if defined(BBMD_ENABLED)
    case PROP_BBMD_ACCEPT_FD_REGISTRATIONS:
        apdu_len = encode_application_boolean(&apdu[0],
            Network_Port_BBMD_Accept_FD_Registrations(currentObject));
        break;

    case PROP_BBMD_BROADCAST_DISTRIBUTION_TABLE:
    case PROP_BBMD_FOREIGN_DEVICE_TABLE:
        /*
            See 12.36.54
                (a) If the Network_Type is IPV4, the current value of the BDT may be read at any time with the Read-BroadcastDistribution-Table BVLL message.
                (b) If this property has no pending changes, reading this property shall return the current value of the BDT.
                (c) If this property has pending changes, reading this property shall return the last value written to the property, and not
                        the current value of the BDT.
                (d) If this property has pending changes, reading the BDT via Read-Broadcast-Table BVLL shall return the current
                        value of the BDT.
                (e) If a list entry contains a host name, then the corresponding entry in the Read-Broadcast-Distribution-Table-Ack
                        BVLL message shall contain the IP address and port of the resolved host name, or X'000000000000' to indicate that
                        the host name has not been resolved.
        */
        // rpdata->error_class = ERROR_CLASS_PROPERTY;
        // rpdata->error_code = ERROR_CODE_READ_ACCESS_DENIED;
        // apdu_len = BACNET_STATUS_ERROR;
        apdu_len = 0;
    break;
#endif

#endif
    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }

    return apdu_len;
}


/* returns true if successful */
bool Network_Port_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
            wp_data->application_data_len, &value);

    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    /*  only array properties can have array options */
    // todo0 - why don't we check for property list here??
    if ((wp_data->array_index != BACNET_ARRAY_ALL) &&
        (wp_data->object_property != PROP_LINK_SPEEDS) &&
        (wp_data->object_property != PROP_IP_DNS_SERVER) &&
        (wp_data->object_property != PROP_IPV6_DNS_SERVER) &&
        (wp_data->object_property != PROP_EVENT_MESSAGE_TEXTS) &&
        (wp_data->object_property != PROP_EVENT_MESSAGE_TEXTS_CONFIG) &&
        (wp_data->object_property != PROP_TAGS)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    NETWORK_PORT_DESCR *currentObject = Network_Port_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Network_Port_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_MAX_MASTER:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if (value.type.Unsigned_Int <= 255) {
                    status = Network_Port_MSTP_Max_Master_Set(
                        currentObject,
                        value.type.Unsigned_Int);
                    if (!status) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        } else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
        break;
    case PROP_MAX_INFO_FRAMES:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if (value.type.Unsigned_Int <= 255) {
                    status = Network_Port_MSTP_Max_Info_Frames_Set(
                        currentObject,
                        value.type.Unsigned_Int);
                    if (!status) {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
    case PROP_BACNET_IP_MODE:
    case PROP_IP_ADDRESS:
    case PROP_IP_DEFAULT_GATEWAY:
    case PROP_BACNET_IP_UDP_PORT:
    case PROP_IP_SUBNET_MASK:
    case PROP_IP_DNS_SERVER:
    case PROP_BBMD_ACCEPT_FD_REGISTRATIONS:
#if 0
    // only if MSTP, and only if slave proxy supported
    // and obsoleted Rev 17
    case PROP_AUTO_SLAVE_DISCOVERY:
#endif
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_RELIABILITY:
    case PROP_NETWORK_TYPE:
    case PROP_PROTOCOL_LEVEL:
    case PROP_NETWORK_NUMBER:
    case PROP_NETWORK_NUMBER_QUALITY:
    case PROP_MAC_ADDRESS:
    case PROP_MAX_APDU_LENGTH_ACCEPTED:
    case PROP_LINK_SPEED:
    case PROP_CHANGES_PENDING:
    case PROP_APDU_LENGTH:
    case PROP_BBMD_FOREIGN_DEVICE_TABLE:
    case PROP_BBMD_BROADCAST_DISTRIBUTION_TABLE:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        break;

    // todo 0 and property lists?
    
    default:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
    }

    return status;
}

/**
 * ReadRange service handler for the BACnet/IP BDT.
 *
 * @param  apdu - place to encode the data
 * @param  apdu - BACNET_READ_RANGE_DATA data
 *
 * @return number of bytes encoded
 */
int Network_Port_Read_Range_BDT(
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest)
{
    return 0;
}


#if ( BACNET_SVC_RR_B == 1 )

int Network_Port_Read_Range_FDT(
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest)
{
    return 0;
}

bool Network_Port_Read_Range(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo)
{
    /* return value */
    bool status = false;

    switch (pRequest->object_property) {
        /* required properties */
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_RELIABILITY:
        case PROP_OUT_OF_SERVICE:
        case PROP_NETWORK_TYPE:
        case PROP_PROTOCOL_LEVEL:
        case PROP_NETWORK_NUMBER:
        case PROP_NETWORK_NUMBER_QUALITY:
        case PROP_CHANGES_PENDING:
        case PROP_APDU_LENGTH:
        case PROP_LINK_SPEED:
        /* optional properties */
        case PROP_MAC_ADDRESS:
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
#if defined(BACDL_MSTP)
        case PROP_MAX_MASTER:
        case PROP_MAX_INFO_FRAMES:
#elif defined(BACDL_BIP)
        case PROP_BACNET_IP_MODE:
        case PROP_IP_ADDRESS:
        case PROP_BACNET_IP_UDP_PORT:
        case PROP_IP_SUBNET_MASK:
        case PROP_IP_DEFAULT_GATEWAY:
        case PROP_IP_DNS_SERVER:
#if defined(BBMD_ENABLED)
        case PROP_BBMD_ACCEPT_FD_REGISTRATIONS:
#endif
#endif
            pRequest->error_class = ERROR_CLASS_SERVICES;
            pRequest->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
            break;

        case PROP_BBMD_BROADCAST_DISTRIBUTION_TABLE:
#if defined(BACDL_BIP) && defined(BBMD_ENABLED)
            pInfo->RequestTypes = RR_BY_POSITION;
            pInfo->Handler = Network_Port_Read_Range_BDT;
            status = true;
#else
            pRequest->error_class = ERROR_CLASS_PROPERTY;
            pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
#endif
            break;

        case PROP_BBMD_FOREIGN_DEVICE_TABLE:
#if defined(BACDL_BIP) && defined(BBMD_ENABLED)
            pInfo->RequestTypes = RR_BY_POSITION;
            pInfo->Handler = Network_Port_Read_Range_FDT;
            status = true;
#else
            pRequest->error_class = ERROR_CLASS_PROPERTY;
            pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
#endif
            break;

        default:
            pRequest->error_class = ERROR_CLASS_PROPERTY;
            pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;

    }

    return status;
}
#endif

/**
 * Initializes the Network Port object data
 */
//void Network_Port_Init(
//    void)
//{
//    unsigned index = 0;
//
//    for (index = 0; index < BACNET_NETWORK_PORTS_MAX; index++) {
//        currentObject->Instance_Number = index + 1;
//        currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;
//        currentObject->Out_Of_Service = false;
//        currentObject->Network_Number = 0;
//        currentObject->Quality = PORT_QUALITY_UNKNOWN;
//        currentObject->APDU_Length = MAX_APDU;
//        currentObject->Link_Speed = 0;
//#if defined(BACDL_MSTP)
//        currentObject->Object_Name = "MS/TP Port";
//        currentObject->Link_Speed = 9600;
//        currentObject->Network_Type = PORT_TYPE_MSTP;
//        currentObject->Network.MSTP.MAC_Address = 255;
//        currentObject->Network.MSTP.Baud_Rate = 9600;
//        currentObject->Network.MSTP.Max_Master = 127;
//        currentObject->Network.MSTP.Max_Info_Frames = 1;
//#elif defined(BACDL_BIP)
//        currentObject->Object_Name = "BACnet/IP Port 1";
//        currentObject->Link_Speed = 0;
//        currentObject->Network_Type = PORT_TYPE_BIP;
//        currentObject->Network.IPv4.IP_Address[0] = 0;
//        currentObject->Network.IPv4.IP_Address[1] = 0;
//        currentObject->Network.IPv4.IP_Address[2] = 0;
//        currentObject->Network.IPv4.IP_Address[3] = 0;
//        currentObject->Network.IPv4.Port = 0xBAC0;
//        currentObject->Network.IPv4.IP_Subnet_Prefix = 24;
//        currentObject->Network.IPv4.IP_Gateway[0] = 0;
//        currentObject->Network.IPv4.IP_Gateway[1] = 0;
//        currentObject->Network.IPv4.IP_Gateway[2] = 0;
//        currentObject->Network.IPv4.IP_Gateway[3] = 0;
//#endif
//    }
//}

#ifdef BACNET_UNIT_TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void test_network_port(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    int test_len = 0;
    BACNET_READ_PROPERTY_DATA rpdata;
    /* for decode value data */
    BACNET_APPLICATION_DATA_VALUE value;
    const int *pRequired = NULL;
    const int *pOptional = NULL;
    const int *pProprietary = NULL;
    unsigned count = 0;
    unsigned index = 0;

    Network_Port_init();
    count = Network_Port_Count();
    for (index = 0; index < count; index++) {
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_NETWORK_PORT;
        rpdata.object_instance = Network_Port_Index_To_Instance(index);
        Network_Port_Property_Lists(&pRequired, &pOptional, &pProprietary);
        while ((*pRequired) != -1) {
            rpdata.object_property = *pRequired;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Network_Port_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
            test_len = bacapp_decode_application_data(
                rpdata.application_data,
                (uint8_t) rpdata.application_data_len,
                &value);
            ct_test(pTest, test_len >= 0);
            if (test_len < 0) {
                printf("<decode failed!>\n");
            }
            pRequired++;
        }
        while ((*pOptional) != -1) {
            rpdata.object_property = *pOptional;
            rpdata.array_index = BACNET_ARRAY_ALL;
            len = Network_Port_Read_Property(&rpdata);
            ct_test(pTest, len != 0);
            test_len = bacapp_decode_application_data(
                rpdata.application_data,
                (uint8_t) rpdata.application_data_len,
                &value);
            ct_test(pTest, test_len >= 0);
            if (test_len < 0) {
                printf("<decode failed!>\n");
            }
            pOptional++;
        }
    }

    return;
}

#ifdef TEST_NETWORK_PORT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Network Port", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, test_network_port);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif
#endif /* TEST */

#endif // ( BACNET_PROTOCOL_REVISION >= 17 )


