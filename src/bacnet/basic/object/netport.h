/**************************************************************************

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
 ****************************************************************************************/

#ifndef NETPORT_H
#define NETPORT_H

#include "bacnet/bacenum.h"
#include "bacnet/rp.h"
#include "bacnet/wp.h"
#include "bacnet/bits/util/BACnetObject.h"
#include "bacnet/readrange.h"
#include "bacnet/bits/util/multipleDatalink.h"
#include "bacnet/bits/util/BACnetObject.h"

#define BIP_DNS_MAX 3
struct bacnet_ipv4_port
{
    uint8_t IP_Address[4];
    uint8_t IP_Subnet_Prefix;
    uint8_t IP_Gateway[4];
    uint8_t IP_DNS_Server[BIP_DNS_MAX][4];
    uint16_t Port;
	BACNET_IP_MODE Mode;
    bool IP_DHCP_Enable;
    uint32_t IP_DHCP_Lease_Seconds;
    uint32_t IP_DHCP_Lease_Seconds_Remaining;
    uint32_t IP_DHCP_Server[4];
    bool IP_NAT_Traversal;
    uint32_t IP_Global_Address[4];
    bool BBMD_Accept_FD_Registrations;
};

struct bacnet_ipv6_port
{
    uint8_t MAC_Address[4];
    uint8_t IP_Address[16];
    uint8_t IP_Prefix;
    uint16_t Port;
};

struct ethernet_port
{
    uint8_t MAC_Address[6];
};

struct mstp_port
{
    uint8_t MAC_Address;
    uint8_t Max_Master;
    uint8_t Max_Info_Frames;
};


typedef struct 
{
    BACNET_OBJECT common ;
    
	BACNET_PORT_TYPE Network_Type;
    uint16_t Network_Number;
    BACNET_PORT_QUALITY Quality;
    uint16_t APDU_Length;
    float Link_Speed;
	bool Changes_Pending;
    union
    {
        struct bacnet_ipv4_port IPv4;
        struct bacnet_ipv6_port IPv6;
        struct ethernet_port Ethernet;
        struct mstp_port MSTP;
    } Network;

} NETWORK_PORT_DESCR ;


void Network_Port_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Network_Port_Object_Name(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

bool Network_Port_Name_Set(
    NETWORK_PORT_DESCR *currentObject,
    char *new_name);

char *Network_Port_Description(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_Description_Set(
    NETWORK_PORT_DESCR *currentObject,
    char *new_name);

//BACNET_RELIABILITY Network_Port_Reliability(
//    uint32_t object_instance);

void Network_Port_Reliability_Set(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_RELIABILITY value);

//bool Network_Port_Out_Of_Service_Get(
//    NETWORK_PORT_DESCR *currentObject );

//bool Network_Port_Out_Of_Service_Set(
//    NETWORK_PORT_DESCR *currentObject,
//    const bool oos_flag);

uint8_t Network_Port_Type(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_Type_Set(
    NETWORK_PORT_DESCR *currentObject,
	BACNET_PORT_TYPE value);

uint16_t Network_Port_Network_Number(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_Network_Number_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint16_t value);

BACNET_PORT_QUALITY Network_Port_Quality(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_Quality_Set(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_PORT_QUALITY value);

bool Network_Port_MAC_Address(
	DEVICE_OBJECT_DATA* pDev,
	NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *mac_address);

// we set the MAC address during datalink setup, and cannot be changed 'simply'
//void Network_Port_MAC_Address_Set(
//    NETWORK_PORT_DESCR *currentObject,
//    uint8_t *mac_src,
//    uint8_t mac_len);

uint16_t Network_Port_APDU_Length(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_APDU_Length_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint16_t value);

uint8_t Network_Port_MSTP_Max_Master(
    NETWORK_PORT_DESCR *currentObject);

bool Network_Port_MSTP_Max_Master_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value);

uint8_t Network_Port_MSTP_Max_Info_Frames(
    NETWORK_PORT_DESCR* currentObject);

bool Network_Port_MSTP_Max_Info_Frames_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value);

float Network_Port_Link_Speed(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_Link_Speed_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint32_t value);

bool Network_Port_IP_Address(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *ip_address);

bool Network_Port_IP_Address_Get(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d);

void Network_Port_IP_Address_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d);

uint8_t Network_Port_IP_Subnet_Prefix(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_IP_Subnet_Prefix_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t value);

bool Network_Port_IP_Subnet(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *subnet_mask);

bool Network_Port_IP_Subnet_Get(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d);

void Network_Port_IP_Subnet_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d);

bool Network_Port_IP_Gateway(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_OCTET_STRING *subnet_mask);

bool Network_Port_IP_Gateway_Get(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d);

void Network_Port_IP_Gateway_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d);

bool Network_Port_IP_DNS_Server(
    NETWORK_PORT_DESCR *currentObject,
    unsigned index,
    BACNET_OCTET_STRING *subnet_mask);

bool Network_Port_IP_DNS_Server_Get(
    NETWORK_PORT_DESCR *currentObject,
    unsigned index,
    uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d);

void Network_Port_IP_DNS_Server_Set(
    NETWORK_PORT_DESCR *currentObject,
    unsigned index,
    uint8_t a, uint8_t b, uint8_t c, uint8_t d);

uint16_t Network_Port_BIP_Port(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_BIP_Port_Set(
    NETWORK_PORT_DESCR *currentObject,
    uint16_t value);

BACNET_IP_MODE Network_Port_BIP_Mode(
    NETWORK_PORT_DESCR *currentObject);

bool Network_Port_BIP_Mode_Set(
    NETWORK_PORT_DESCR *currentObject,
    BACNET_IP_MODE value);

bool Network_Port_BBMD_Accept_FD_Registrations(
    NETWORK_PORT_DESCR *currentObject);

bool Network_Port_BBMD_Accept_FD_Registrations_Set(
    NETWORK_PORT_DESCR *currentObject,
    bool value);

bool Network_Port_Changes_Pending(
    NETWORK_PORT_DESCR *currentObject);

void Network_Port_Changes_Pending_Set(
    NETWORK_PORT_DESCR *currentObject,
    bool flag);

bool Network_Port_Valid_Instance(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance);

unsigned Network_Port_Count(
    DEVICE_OBJECT_DATA *pDev);

uint32_t Network_Port_Index_To_Instance(
    DEVICE_OBJECT_DATA *pDev,
    unsigned find_index);

unsigned Network_Port_Instance_To_Index(
    uint32_t object_instance);

bool Network_Port_Set_Network_Port_Instance_ID(
    NETWORK_PORT_DESCR* currentObject,
    uint32_t object_instance);

int Network_Port_Read_Range_BDT(
	DEVICE_OBJECT_DATA* pDev,
	uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest);

int Network_Port_Read_Range_FDT(
    DEVICE_OBJECT_DATA* pDev,
    uint8_t* apdu,
    BACNET_READ_RANGE_DATA* pRequest);

BACNET_OBJECT* Network_Port_Create(
	DEVICE_OBJECT_DATA* pDev,
	const uint32_t instance,
	const char* name);

bool Network_Port_Read_Range(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo);

bool Network_Port_Delete(
    uint32_t object_instance);

void Network_Port_Cleanup(
    void);

void Network_Port_Init(
    void);

/* handling for read property service */
int Network_Port_Read_Property(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_READ_PROPERTY_DATA * rpdata);

/* handling for write property service */
bool Network_Port_Write_Property(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_WRITE_PROPERTY_DATA * wp_data);

NETWORK_PORT_DESCR *Network_Port_Instance_To_Object(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance);

// PORT_SUPPORT *FindFirstActiveMSTPport(void);

#endif
