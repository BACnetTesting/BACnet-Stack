/**************************************************************************
 *
 * Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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

#ifndef BACTEXT_H
#define BACTEXT_H

/* tiny implementations have no need to print */
#if PRINT_ENABLED
#define BACTEXT_PRINT_ENABLED
#else
#ifdef TEST
#define BACTEXT_PRINT_ENABLED
#endif
#endif

#include <stdbool.h>
#include "bacnet/bacenum.h"
#include "bacnet/bacdef.h"

const char *bactext_confirmed_service_name(
    unsigned index);

const char *bactext_unconfirmed_service_name(
    unsigned index);

const char *bactext_application_tag_name(
    unsigned index);
    
bool bactext_application_tag_index(
    const char *search_name,
    unsigned *found_index);

const char *bactext_object_type_name(
    unsigned index);

bool bactext_object_type_index(
    const char *search_name,
    unsigned *found_index);

bool bactext_object_type_enum(
    const char *search_name,
    BACNET_OBJECT_TYPE *found_type);

bool bactext_object_type_enum_by_acronym(
	const char *search_name,
	BACNET_OBJECT_TYPE *found_type) ;

const char *bactext_object_type_acronym_by_enum(
    BACNET_OBJECT_TYPE type);

const char *bactext_notify_type_name(
    unsigned index);

const char *bactext_event_type_name(
    unsigned index);

const char *bactext_property_name(
    unsigned index);

const char *bactext_property_name_default(
    unsigned index,
    const char *default_string);

bool bactext_property_index(
    const char *search_name,
    unsigned *found_index);

const char *bactext_engineering_unit_name(
    unsigned index);

bool bactext_engineering_unit_index(
    const char *search_name,
    unsigned *found_index);

bool bactext_engineering_unit_enum(
    const char *search_name,
    BACNET_ENGINEERING_UNITS *found_units);

bool bactext_engineering_unit_enum_by_synonym(
		const char *search_name,
		BACNET_ENGINEERING_UNITS *found_unit_enum) ;

const char *bactext_reject_reason_name(
    BACNET_REJECT_REASON index);

const char *bactext_abort_reason_name(
    unsigned index);

const char *bactext_error_class_name(
    unsigned index);

const char *bactext_error_code_name(
    unsigned index);

unsigned bactext_property_id(
    const char *name);

const char *bactext_month_name(
    unsigned index);

const char *bactext_week_of_month_name(
    unsigned index);

const char *bactext_day_of_week_name(
    unsigned index);

const char *bactext_event_state_name(
    unsigned index);

const char *bactext_binary_present_value_name(
    unsigned index);

const char *bactext_binary_polarity_name(
    unsigned index);

bool bactext_binary_present_value_index(
    const char *search_name,
    unsigned *found_index);

const char *bactext_reliability_name(
    unsigned index);

const char *bactext_device_status_name(
    unsigned index);

const char *bactext_segmentation_name(
    unsigned index);
    
bool bactext_segmentation_index(
    const char *search_name,
    unsigned *found_index);
    
const char *bactext_node_type_name(
    unsigned index);

const char *bactext_character_string_encoding_name(
    unsigned index);

const char *bactext_event_transition_name(
    unsigned index);

bool bactext_event_transition_index(
    const char *search_name,
    unsigned *found_index);

const char *bactext_days_of_week_name(
    unsigned index);

bool bactext_days_of_week_index(
    const char *search_name,
    unsigned *found_index);

const char *bactext_network_layer_msg_name(
    unsigned index);

const char *bactext_life_safety_state_name(
    unsigned index);

const char *bactext_device_communications_name(
    unsigned index);
        
const char *bactext_lighting_operation_name(
    unsigned index);

const char *bactext_lighting_in_progress(
    unsigned index);

const char *bactext_lighting_transition(
    unsigned index);

const char *bactext_bacnet_mac_address(
    char *tbuf, 
    BACNET_MAC_ADDRESS *addr);

const char *bactext_bacnet_global_address(
    char *tbuf, 
    BACNET_GLOBAL_ADDRESS *addr);

const char *bactext_bacnet_path(
    char *tbuf,
    BACNET_PATH *addr);

#endif // BACTEXT_H
