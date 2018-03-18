/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#pragma once

// actually, this is required for microsoft C (perhaps not C++, still to check) #ifndef _MSC_VER
#include <stdbool.h>
//#endif

#include <stdint.h>

#define DELIBERATELY_FAIL_MALLOCS       0         // x, where 1-in-X will fail
#define EMMTRACELOG                     0

#if ( DELIBERATELY_FAIL_MALLOCS != 0 )
#include <stdlib.h>
#endif

bool emm_init(void);
bool emm_check_alloc_three(void *p1, void *p2, void *p3);
bool emm_check_alloc_two(void *p1, void *p2);
void emm_free(void *p1);
void emm_free_two(void *p1, void *p2);
void emm_free_three(void *p1, void *p2, void *p3);
void emm_free_provisional(void *p);

#if (EMMTRACELOG == 1 )
void *emm_sys_malloc(uint8_t tag, uint16_t size);
void *emm_sys_safe_malloc(uint8_t tag, uint16_t size);            // no 'deliberately fail mallocs' allowed here.
void *emm_sys_cmalloc(uint8_t tag, uint16_t size);
#define emm_dmalloc(tag,size) emm_sys_malloc(tag,size)
#define emm_dcalloc(tag,size) emm_sys_calloc(tag,size)
#define emm_smalloc(tag,size) emm_sys_safe_malloc(tag,size)
void DumpEmmTraceLog(void);
#else
void *emm_sys_malloc(uint16_t size);
void *emm_sys_safe_malloc(uint16_t size);
#define emm_dmalloc(tag,size) emm_sys_malloc(size)
#define emm_smalloc(tag,size) emm_sys_safe_malloc(size)
#endif

#if ( EMM_MEM_TEST == 1 )		// Set this project wide, and then running emulation will launch mem test
void test_mem(void);
#endif

extern bool emm_library_initialized;



