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
*****************************************************************************************
*
*   Modifications Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   Oct 14, 2018    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*   For more information: info@bac-test.com
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
*  2018.10.14 EKH Diffed in hints from Analog Input for future reference
*
****************************************************************************************/

#ifndef PIV_H
#define PIV_H

#include "config.h"
//#include <stdbool.h>
//#include <stdint.h>
//#include "bacdef.h"
//#include "bacerror.h"
//#include "wp.h"
//#include "rp.h"

typedef struct positiveinteger_value_descr {
    bool Out_Of_Service:1;
    uint32_t Present_Value;     // btc todo - test 32 bitiveness
    uint32_t COV_Increment;     // btc todo - test 32 bitiveness
    uint16_t Units;
} POSITIVEINTEGER_VALUE_DESCR;


void PositiveInteger_Value_Property_Lists(const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool PositiveInteger_Value_Valid_Instance(uint32_t object_instance);

unsigned PositiveInteger_Value_Count(void);

uint32_t PositiveInteger_Value_Index_To_Instance(unsigned index);

unsigned PositiveInteger_Value_Instance_To_Index(uint32_t object_instance);

bool PositiveInteger_Value_Object_Name(uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

int PositiveInteger_Value_Read_Property(BACNET_READ_PROPERTY_DATA *
    rpdata);

bool PositiveInteger_Value_Write_Property(BACNET_WRITE_PROPERTY_DATA *
    wp_data);

bool PositiveInteger_Value_Present_Value_Set(uint32_t object_instance,
    uint32_t value,
    uint8_t priority);
uint32_t PositiveInteger_Value_Present_Value(uint32_t object_instance);

bool PositiveInteger_Value_Change_Of_Value(uint32_t instance);

void PositiveInteger_Value_Change_Of_Value_Clear(uint32_t instance);

bool PositiveInteger_Value_Encode_Value_List(uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

char *PositiveInteger_Value_Description(uint32_t instance);

bool PositiveInteger_Value_Description_Set(uint32_t instance,
    char *new_name);

bool PositiveInteger_Value_Out_Of_Service(uint32_t instance);

void PositiveInteger_Value_Out_Of_Service_Set(uint32_t instance,
    bool oos_flag);

#if (INTRINSIC_REPORTING_B == 1)
void PositiveInteger_Value_Intrinsic_Reporting(uint32_t object_instance);
#endif

void PositiveInteger_Value_Init(void);

#ifdef TEST
#include "ctest.h"
    void testPositiveInteger_Value(Test * pTest);
#endif

#endif
