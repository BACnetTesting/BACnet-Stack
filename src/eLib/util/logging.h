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

#pragma once

#include "eLib/util/llist.h"

/* 
    This (extensible) logging approach is based on the ability for multiple observers to register
    with the logging dispatcher.
    
    A 'log item' is a text string, of unlimited length (actually, ethernet packet size), along with 
    a priority, and a subsystem.

    A list of registered observers, along with their subsystem(s) of interest, and the priority is
    maintained.

    An observer may register for different subsystems at different priorities. Typically 'all'
    subsystems at lowest priority, and then the noise is then filtered out by eliminating 
    the subsystems/raising priorities one by one.

    The code logs with a given subsystem at a given priority.

    Input devices can also register. These can be keyboards, BTA, telnet, etc.
    
    */

// Subsystems. Max 32.

typedef enum
{
    BL_MSTP = (1<<0),
    BL_BBMD = (1 << 1),
    BL_MEMORY = (1 << 2),
    BL_QUEUES = (1 << 3),
    BL_IP = (1 << 4),
    BL_READ = (1 << 5),
    BL_WRITE = (1 << 6),
    BL_CONSOLE = (1 << 7),
    BL_SYSLOG = (1 << 8),
} BL_Subsystem ;


// Logging levels. Should conform to linux syslog levels for sanity's sake.

typedef enum
{
    BLL_Debug,
    BLL_Release,
    BLL_None
} BL_LEV ;

typedef struct
{
    LLIST_LB        llcb;
    BL_Subsystem    subsystem;
    BL_LEV          level;
    char            *message;
} BL_LOG_ENTRY ;


void log_printf(const char *fmt, ...);
void log_puts(const char *msg );
void log_putc(const char c);
void log_puts_toFile(const char *msg);

void log_lev_printf( const BL_LEV lev, const char *fmt, ...);

void sys_log_lev_trace(const BL_LEV level, const char *file, const int line);
void sys_log_assert(int assrt, const char *file, const int line);

#define log_lev_trace(a)    sys_log_lev_trace(a, __FILE__, __LINE__ )
#define log_assert(a)       sys_log_assert(a, __FILE__, __LINE__ )
// in bitsDebug.h #define panic()             log_printf("Panic %d, %s\n", __LINE__, __FILE__ )

