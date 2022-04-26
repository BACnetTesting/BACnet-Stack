/**************************************************************************
 *
 *   Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
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

#ifdef CONFIG_H
#error - We have a problem. Please see my notes in config.h
#endif

#include "osLayer.h"

#ifndef CONFIG_PROJECT_DEFAULT_H
#define CONFIG_PROJECT_DEFAULT_H

#ifndef BACNET_USE_COMMANDABLE_VALUE_OBJECTS 
#define BACNET_USE_COMMANDABLE_VALUE_OBJECTS        1
#endif

#ifndef BACNET_USE_MAX_MIN_RESOLUTION_PRESENT_VALUE
#define BACNET_USE_MAX_MIN_RESOLUTION_PRESENT_VALUE 1
#endif


#endif

