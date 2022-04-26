/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
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

// actually, this is required for microsoft C (perhaps not C++, still to check)
// #ifndef _MSC_VER
#include <stdbool.h>
//#endif

#include <stdint.h>

#define DELIBERATELY_FAIL_MALLOCS       0         // x, where 1-in-X will fail
#define EMMTRACELOG                     0

#if (DELIBERATELY_FAIL_MALLOCS != 0)
#include <stdlib.h>
#endif

#if ( BAC_DEBUG == 1 )
bool emm_validate(void* p1);
#endif 

bool emm_init(void);
bool emm_check_alloc_three(void *p1, void *p2, void *p3);
bool emm_check_alloc_two(void *p1, void *p2);
bool emm_check_alloc(void* p1);
void emm_free(void *p1);
void emm_free_two(void *p1, void *p2);
void emm_free_three(void *p1, void *p2, void *p3);
void emm_free_provisional(void *p);

#if (EMMTRACELOG == 1)
void *emm_sys_malloc(uint8_t tag, uint16_t size);
void *emm_sys_safe_malloc(uint8_t tag, uint16_t size);            // no 'deliberately fail mallocs' allowed here.
void *emm_sys_cmalloc(uint8_t tag, uint16_t size);
#define emm_dmalloc(tag,size) emm_sys_malloc(tag,size)
#define emm_dcalloc(tag,size) emm_sys_calloc(tag,size)
#define emm_smalloc(tag,size) emm_sys_safe_malloc(tag,size)
void DumpEmmTraceLog(void);
#else
void *emm_sys_malloc(uint16_t size);
void *emm_sys_calloc(uint16_t size);
// 'safe' means safe from interference from memory starvation testing scenarios
void *emm_sys_safe_malloc(uint16_t size);
void *emm_sys_safe_calloc(uint16_t size);
#define emm_dmalloc(tag,size) emm_sys_malloc(size)
#define emm_dcalloc(tag,size) emm_sys_calloc(size)
#define emm_smalloc(tag,size) emm_sys_safe_malloc(size)
#define emm_scalloc(tag,size) emm_sys_safe_calloc(size)
#define emm_calloc(notused,size) emm_sys_safe_calloc(size)
#endif

#if ( EMM_MEM_TEST == 1 )		// Set this project wide, and then running emulation will launch mem test
void test_mem(void);
#endif

extern bool emm_library_initialized;

