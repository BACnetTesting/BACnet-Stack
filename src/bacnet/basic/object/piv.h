/**************************************************************************
 *
 * Copyright (C) 2015 Nikola Jelic <nikola.jelic@euroicc.com>
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

#ifndef PIV_H
#define PIV_H

#include <stdbool.h>
#include <stdint.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacerror.h"
#include "bacnet/wp.h"
#include "bacnet/rp.h"

#ifdef __cplusplus_disable
extern "C" {
#endif /* __cplusplus */

    typedef struct positiveinteger_value_descr {
        bool Out_Of_Service:1;
        uint32_t Present_Value;
        uint16_t Units;
    } POSITIVEINTEGER_VALUE_DESCR;


    BACNET_STACK_EXPORT
    void PositiveInteger_Value_Property_Lists(const int **pRequired,
        const int **pOptional,
        const int **pProprietary);
    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Valid_Instance(uint32_t object_instance);

unsigned Positive_Integer_Value_Count(
    DEVICE_OBJECT_DATA* pDev);

    BACNET_STACK_EXPORT
    uint32_t PositiveInteger_Value_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index);
    
    BACNET_STACK_EXPORT
    unsigned PositiveInteger_Value_Instance_To_Index(
    uint32_t object_instance);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Object_Name(
        DEVICE_OBJECT_DATA* pDev,
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);

    BACNET_STACK_EXPORT
    int PositiveInteger_Value_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA *        rpdata);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Write_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA *
        wp_data);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Present_Value_Set(
        uint32_t object_instance,
        uint32_t value,
        uint8_t priority);

    BACNET_STACK_EXPORT
    uint32_t PositiveInteger_Value_Present_Value(
    uint32_t object_instance);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Change_Of_Value(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t instance);

    BACNET_STACK_EXPORT
    void PositiveInteger_Value_Change_Of_Value_Clear(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t instance);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Encode_Value_List(
    uint32_t object_instance,
        DEVICE_OBJECT_DATA* pDev,
        BACNET_PROPERTY_VALUE * value_list);

    BACNET_STACK_EXPORT
    char *PositiveInteger_Value_Description(
        DEVICE_OBJECT_DATA* pDev,
        uint32_t instance);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Description_Set(
        DEVICE_OBJECT_DATA* pDev,
        uint32_t instance,
        char *new_name);

    BACNET_STACK_EXPORT
    bool PositiveInteger_Value_Out_Of_Service(uint32_t instance);
    
    BACNET_STACK_EXPORT
    void PositiveInteger_Value_Out_Of_Service_Set(uint32_t instance,
        bool oos_flag);

    /* note: header of Intrinsic_Reporting function is required
       even when INTRINSIC_REPORTING is not defined */
    BACNET_STACK_EXPORT
    void PositiveInteger_Value_Intrinsic_Reporting(uint32_t object_instance);

    BACNET_STACK_EXPORT
    void PositiveInteger_Value_Init(void);

#ifdef BAC_TEST
#include "ctest.h"
    BACNET_STACK_EXPORT
    void testPositiveInteger_Value(Test * pTest);
#endif

#ifdef __cplusplus_disable
}
#endif /* __cplusplus */
#endif
