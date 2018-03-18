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
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "address.h"
#include "bacdef.h"
#include "bacapp.h"
#include "datalink.h"
#include "bacdcode.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "dlmstp.h"
#include "CEDebug.h"
#include "asic_objects.h"
/* me */
#include "netport.h"

struct bacnet_ipv4_port {
    uint8_t Address[4];
    uint8_t Subnet;
    uint16_t Port;
};

struct bacnet_ipv6_port {
    uint8_t Address[16];
    uint16_t Broadcast;
    uint16_t Port;
};

struct ethernet_port {
    uint8_t Address[6];
};

struct mstp_port {
    uint8_t Address;
    uint32_t Baud_Rate;
};

struct object_data {
    uint32_t Instance_Number;
    char *Object_Name;
    uint8_t Reliability;
    bool Out_Of_Service;
    NETWORK_TYPE Network_Type;
    uint16_t    apdu_length;
    uint16_t    networkNumber ;
    union {
        struct bacnet_ipv4_port IPv4;
        struct bacnet_ipv6_port IPv6;
        struct ethernet_port Ethernet;
        struct mstp_port MSTP;
    } Network;
};

#define BACNET_NETWORK_PORTS_MAX 2

struct object_data Object_List[BACNET_NETWORK_PORTS_MAX];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Network_Port_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_STATUS_FLAGS,
    PROP_RELIABILITY,
    PROP_OUT_OF_SERVICE,
    PROP_NETWORK_TYPE,
    PROP_PROTOCOL_LEVEL,
    PROP_NETWORK_NUMBER,
    PROP_NETWORK_NUMBER_QUALITY,
    PROP_CHANGES_PENDING,
    PROP_APDU_LENGTH,
    PROP_LINK_SPEED,
    -1
};

static const int Network_Port_Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_MAC_ADDRESS,
    PROP_MAX_APDU_LENGTH_ACCEPTED,
    PROP_MAX_MASTER,
    PROP_MAX_INFO_FRAMES,
    -1
};

static const int Network_Port_Properties_Proprietary[] = {
    -1
};

void Network_Port_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired) {
        *pRequired = Network_Port_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Network_Port_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Network_Port_Properties_Proprietary;
    }

    return;
}

/**************************************************************************
* Description: Determines the object name
* Returns: true if name is valid, and name is returned in BACnet
*   Character String format
* Notes: none
**************************************************************************/
bool Network_Port_Object_Name(
    uint32_t instance,
    BACNET_CHARACTER_STRING *object_name)
{
    static char name[50];
    sprintf(name, "Network Port %d - ", instance);
    switch ( Object_List[instance].Network_Type )
    {
    case NETWORK_TYPE_MSTP:
      strcat ( name, "MS/TP");
      break;
    case NETWORK_TYPE_IPV4:
      strcat ( name, "IPv4");
      break;
    default:
      panic();
      break;
    }
      
    return characterstring_init_ansi(object_name, name );
}


/**************************************************************************
* Description: Determines the object instance is valid
* Returns: true if valid
* Notes: none
**************************************************************************/
bool Network_Port_Valid_Instance(
    uint32_t object_instance)
{
    return (object_instance < BACNET_NETWORK_PORTS_MAX );
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Network_Port_Count(
    void)
{
    return BACNET_NETWORK_PORTS_MAX;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Network_Port_Index_To_Instance(
    unsigned find_index)
{
    return find_index;
}


bool Network_Port_Set_Network_Port_Instance_ID(
    uint32_t netport_id)
{
    bool status = true; /* return value */

    if (netport_id <= BACNET_MAX_INSTANCE) {
        //Network_Port_Instance_Number = netport_id;
        //// Write the new ID to SEEPROM
        //SEEPROM_Bytes_ReadWrite (1, NV_SEEPROM_NETPORT_0, (uint8_t *)&netport_id, 4);
    } else
        status = false;

    return status;
}


int Network_Port_Read_Property(
    BACNET_READ_PROPERTY_DATA *rpdata)
{

    int apdu_len = 0;   /* return value */

    //char text_string[32] = { "" };
    BACNET_BIT_STRING bit_string;
    //BACNET_OCTET_STRING octet_string;
    BACNET_CHARACTER_STRING char_string;
    //BACNET_OCTET_STRING octet_string;
    //BACNET_DATE bdate;
    //BACNET_TIME btime;
    uint8_t *apdu = NULL;
    //uint8_t signature[16] = {0};
    //uint32_t file_size = 0;
    //uint32_t error_code = 0;
    //unsigned index = 0;
    //bool status;

    if ((rpdata == NULL) ||
        (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_NETWORK_PORT,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:  // todo5
            Network_Port_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_NETWORK_PORT);
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_RELIABILITY:
            apdu_len = encode_application_enumerated(&apdu[0], RELIABILITY_NO_FAULT_DETECTED);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        case PROP_NETWORK_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], Object_List[rpdata->object_instance].Network_Type );
            break;
        case PROP_NETWORK_NUMBER_QUALITY:
            apdu_len = encode_application_enumerated(&apdu[0], NETWORK_NUMBER_QUALITY_UNKNOWN);
            break;
            
        case PROP_NETWORK_NUMBER:
            apdu_len = encode_application_unsigned(&apdu[0], Object_List[rpdata->object_instance].networkNumber );
            break;
            
        case PROP_BACNET_IP_UDP_PORT :
          if ( Object_List[rpdata->object_instance].Network_Type == NETWORK_TYPE_IPV4 )
          {
            apdu_len = encode_application_unsigned(&apdu[0], Object_List[rpdata->object_instance].Network.IPv4.Port );
          }
          else 
          {
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
          }
          break;

        case PROP_MAC_ADDRESS:
            apdu_len = encode_application_unsigned(&apdu[0], gSYS_nv->BACnetMSTP_MAC);
/*
            // todo EKH - review proposed changes for BACnet/IP option (future)
            // and I am sure this is meant to be an octet array. todo 1
            if (gSYS_nv->BACnetMSTP_Enable) {
              apdu_len = encode_application_unsigned(&apdu[0], gSYS_nv->BACnetMSTP_MAC);
            } else {
              BACNET_PATH           path;
              BACNET_OCTET_STRING   string;
               
              bip_get_my_address(&path);
              octetstring_init(&string, (uint8_t*)&path.mac, path.mac_len);
              apdu_len += encode_application_octet_string(&apdu[0], &string);
            }
*/
            break;

        case PROP_PROTOCOL_LEVEL:
            apdu_len = encode_application_enumerated(&apdu[0], BACNET_PROTOCOL_LEVEL_APPLICATION);
            break;

        case PROP_APDU_LENGTH:
            apdu_len = encode_application_unsigned(&apdu[0], Object_List[rpdata->object_instance].apdu_length);
            break;

        case PROP_MAX_MASTER:
          if (Object_List[rpdata->object_instance].Network_Type == NETWORK_TYPE_MSTP)
          {
            apdu_len = encode_application_unsigned(&apdu[0], dlmstp_max_master());
          }
          else 
          {
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
          }
          break;

        case PROP_MAX_INFO_FRAMES:
            if (Object_List[rpdata->object_instance].Network_Type == NETWORK_TYPE_MSTP)
            {
                apdu_len = encode_application_unsigned(&apdu[0], dlmstp_max_info_frames());
            }
            else
            {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
                apdu_len = BACNET_STATUS_ERROR;
            }
            break;

        case PROP_LINK_SPEED:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        case PROP_CHANGES_PENDING:
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        //case PROP_COMMAND_SEND:
        //    break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    return apdu_len;
}

/**************************************************************************
* Description: Provides a WriteProperty service
* Returns: returns true if successful
* Notes: none
**************************************************************************/
bool Network_Port_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!Network_Port_Valid_Instance(wp_data->object_instance)) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if (wp_data->array_index != BACNET_ARRAY_ALL) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    
    
    /* FIXME: len < application_data_len: more data? */
    switch (wp_data->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            if (value.tag == BACNET_APPLICATION_TAG_OBJECT_ID) {
                if ((value.type.Object_Id.type == OBJECT_NETWORK_PORT) &&
                    (Network_Port_Set_Network_Port_Instance_ID(value.type.Object_Id.instance))) {
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
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_DESCRIPTION:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
            
        case PROP_MAC_ADDRESS:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if ((value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= 127)) {
                      if ( Object_List[wp_data->object_instance].Network_Type == NETWORK_TYPE_MSTP )
                      {
                      dlmstp_set_mac_address(value.type.Unsigned_Int);
                      status = true;
                      _ALO_SAVE_ = true ;
                      }
                      else 
                      {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
                      }
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
            
        case PROP_NETWORK_NUMBER:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if ((value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= 65534 )) {
                      Object_List[wp_data->object_instance].networkNumber = value.type.Unsigned_Int ;
                      if ( Object_List[wp_data->object_instance].Network_Type == NETWORK_TYPE_MSTP )
                      {
                        gSYS_nv->BACnetMSTP_Network = value.type.Unsigned_Int ;
                      }
                      else 
                      {
                        gSYS_nv->BACnetIP_Network = value.type.Unsigned_Int ;
                      }
                      _ALO_SAVE_ = true ;
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
            
        case PROP_BACNET_IP_UDP_PORT :
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if ((value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= 65534 )) {
                      if ( Object_List[wp_data->object_instance].Network_Type == NETWORK_TYPE_IPV4 )
                      {
                        Object_List[wp_data->object_instance].Network.IPv4.Port  = value.type.Unsigned_Int ;
                        gSYS_nv->BACnetIP_Port = value.type.Unsigned_Int ;     
                        _ALO_SAVE_ = true ;
                        status = true;
                      }
                      else
                      {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
                      }
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
          break;
            
        case PROP_MAX_MASTER:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if ((value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= 127)) {
                      if ( Object_List[wp_data->object_instance].Network_Type == NETWORK_TYPE_MSTP )
                      {
                      dlmstp_set_max_master(value.type.Unsigned_Int);
                      status = true;
                      _ALO_SAVE_ = true ;
                      }
                      else 
                      {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
                      }
                } else {
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
                      if ( Object_List[wp_data->object_instance].Network_Type == NETWORK_TYPE_MSTP )
                      {
                      dlmstp_set_max_info_frames(value.type.Unsigned_Int);
                      status = true;
                      _ALO_SAVE_ = true ;
                      }
                      else 
                      {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
                      }
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;

    }

    return status;
}

void Network_Port_Init(
    void)
{
  // todo Paul - 
  if (gSYS_nv->BACnetIP_Port == 0)
  {
    // use the universal default BACnet/IP port (47808)
    gSYS_nv->BACnetIP_Port = 0xBAC0;
    // use ASI "system bus" address as default BACnet/IP network id
    gSYS_nv->BACnetIP_Network = gSYS_nv->SystemAddress;
    // use product model number (9520, 9540, etc...) as default network id
    gSYS_nv->BACnetMSTP_Network = gSYS_nv->ModelNumber;
    gSYS_nv->BACnetMSTP_NetworkLocal1 = gSYS_nv->ModelNumber + 1;
    gSYS_nv->BACnetMSTP_NetworkLocal2 = gSYS_nv->ModelNumber + 2;
    // save the changes
    _ALO_SAVE_ = true;
  }

  Object_List[0].Network_Type = NETWORK_TYPE_MSTP ;
  Object_List[0].apdu_length = MAX_APDU_MSTP ;
  Object_List[0].networkNumber = gSYS_nv->BACnetMSTP_Network ;
  
  Object_List[1].Network_Type = NETWORK_TYPE_IPV4 ;
  Object_List[1].apdu_length = MAX_APDU_IP;
  Object_List[1].networkNumber = gSYS_nv->BACnetIP_Network ;
  Object_List[1].Network.IPv4.Port = gSYS_nv->BACnetIP_Port ;
}
