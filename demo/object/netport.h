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

#ifndef NETPORT_H
#define NETPORT_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacenum.h"
#include "apdu.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void Network_Port_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Network_Port_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Network_Port_Name_Set(
        uint32_t object_instance,
        char *new_name);

    char *Network_Port_Description(
        uint32_t instance);
    bool Network_Port_Description_Set(
        uint32_t instance,
        char *new_name);

    bool Network_Port_Out_Of_Service(
        uint32_t instance);
    void Network_Port_Out_Of_Service_Set(
        uint32_t instance,
        bool oos_flag);

    bool Network_Port_Valid_Instance(
        uint32_t object_instance);

    unsigned Network_Port_Count(
        void);

    uint32_t Network_Port_Index_To_Instance(
        unsigned find_index);

    bool Network_Port_Create(
        uint32_t object_instance);
    bool Network_Port_Delete(
        uint32_t object_instance);
    void Network_Port_Cleanup(
        void);
    void Network_Port_Init(
        void);

    /* handling for read property service */
    int Network_Port_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    /* handling for write property service */
    bool Network_Port_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
