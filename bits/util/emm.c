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

/*
 (Deeply) Embedded Memory Manager (EMM) for small systems.

 Link to notes (Version 1) : http://goo.gl/wiyFTj
 Link to notes (Version 2) : See notes in pr1124

 Designed using "Buddy Memory Allocator" so that internal fragmentation will not slowly ossify available memory, by this
 I mean that some mallocs() will be unable to allocate a block even though plenty of free memory seems to be available..
 the free memory just has fragmented over time and there  is no longer enough CONTIGUOUS memory availabe memory
 to allocate - something I find happens a lot with embedded systems with runtimes extending to years...

 Although some memory is wasted by the fact that only fixed power-of-two blocks are allocatable, the control array
 is 2 bytes * the number of smallest blocks - very efficient.

 See: http://en.wikipedia.org/wiki/Buddy_memory_allocation

 */

#include <stddef.h>

// for linux only
#include <malloc.h>

#include "config.h"
// now in emm.h #define DELIBERATELY_FAIL_MALLOCS   0  
// in emm.h #define EMMTRACELOG                 1       // alloc checking 

#if defined ( _MSC_VER  )       // Microsoft C

#define MSC_MALLOC      1       // (define for MSC leak test, undef for testing emm)
#define EMM_MEM_TEST    1

#if ( MSC_MALLOC == 1 )
#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <memory.h>
#include <stdlib.h>
#endif // MSC_MALLOC

#else // _MSC_VER

#endif // _MSC_VER

#include "debug.h"
#include "llist.h"
#include "emm.h"
#include "btaDebug.h"
#include "ese.h"
#include "bitsDebug.h"

#if ( BAC_DEBUG == 1 )
#define CHECK_FENCES    0   // we get to choose
#else
#define CHECK_FENCES    0
#endif


#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
#define OS_CREATERSEMA(a)
#define OS_Use(a)
#define OS_Unuse(a)
#endif

#if ( CHECK_FENCES == 1 )
typedef struct {
  uint8_t   signature ;
  uint8_t   tag ;
  uint16_t  length ;
} EMM_FENCE ;
#endif

bool emm_check_alloc(void *p1)
{
    if (p1 == NULL)
    {
        return false;
    }
    return true;
}

bool emm_check_alloc_two(void *p1, void *p2)
{
    if (p1 == NULL)
    {
        if (p2 != NULL) emm_free(p2);
        return false;
    }
    if (p2 == NULL)
    {
        emm_free(p1);
        return false;
    }
    return true;
}

bool emm_check_alloc_three(void *p1, void *p2, void *p3)
{
    if (p1 == NULL)
    {
        emm_free_two(p2, p3);
        return false;
    }
    if (p2 == NULL)
    {
        emm_free(p1);
        emm_free_provisional(p3);
        return false;
    }
    if (p3 == NULL)
    {
        emm_free(p1);
        emm_free(p2);
        return false;
    }
    return true;
}

void emm_free_provisional(void *p)
{
    if (p != NULL)
    {
        emm_free(p);
    }
}

// Malloc cleanup if pointers are not null
void emm_free_two(void *p1, void *p2)
{
    if (p1 != NULL) emm_free(p1);
    if (p2 != NULL) emm_free(p2);
}

void emm_free_three(void *p1, void *p2, void *p3)
{
    if (p1 != NULL) emm_free(p1);
    emm_free_two(p2, p3);
}



#if (EMMTRACELOG == 1 )
typedef struct
{
    uint8_t     tag;
    void        *ptr;
    uint16_t    len;
} TRACELOGITEM;

#define MX_TRACELOG     300 

TRACELOGITEM tracelog[MX_TRACELOG];
static uint16_t tlAvailable;
static bool wasfilled;
static uint16_t allocCount;
static uint16_t allocCountHW;
static uint16_t allocCountLW;
static uint32_t totMalloc ;
static uint32_t mxMalloc ;
static uint8_t  uxFails ;

#if ! defined ( _MSC_VER  ) && ! defined ( __GNUC__ )
static OS_RSEMA tracelogSema ;
#endif

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
void DumpEmmTraceLog(void)
{
    printf("\n\r\nEMM Trace log:");
    for (int i = 0; i < tlAvailable; i++)
    {
        printf("\n\r  %c  %3d", tracelog[i].tag, tracelog[i].len);
    }
    printf("\n\r Total Mallocs: %d", allocCount);
    printf("\n\r");
}
#endif
#endif


#if ( EMMTRACELOG == 1 )
void SendBTAmemoryStats(void)
{
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
    DumpEmmTraceLog();
#endif
    uint16_t datalen = BX_DATAOFFSET;

    uint8_t *outbuf = (uint8_t *)emm_dmalloc('g', MX_BTA_BUFFER);
    if (outbuf == NULL) return;
    memset(outbuf, 0, MX_BTA_BUFFER);

    // build the memory stat message

    datalen += encodeUInt16(&outbuf[datalen], allocCount);      // blocks allocated
    datalen += encodeUInt16(&outbuf[datalen], allocCountHW );    // blocks allocated - High water
    datalen += encodeUInt16(&outbuf[datalen], allocCountLW);     // blocks allocated - Low water
    outbuf[datalen++] = uxFails ; 
    datalen += encodeUInt32(&outbuf[datalen], totMalloc );       // total memory allocated over time
    datalen += encodeUInt32(&outbuf[datalen], mxMalloc );        // HW memory allocated
    datalen += encodeUInt16(&outbuf[datalen], tlAvailable);      // tracelog records available

    for (uint16_t i = tlAvailable; i > 0; i--)
    {
        if ( datalen > MX_BTA_BUFFER - 10 )
        {
          ese_enqueue_once( ese024_18_BTA_buffer_overrun ) ;
          emm_free ( outbuf ) ;
          return;
        }
        outbuf[datalen++] = tracelog[i - 1].tag;
        datalen += encodeUInt16(&outbuf[datalen], tracelog[i - 1].len);
    }

    PrepareBTAheader( BTAmsgType_MemoryAllocStat, outbuf ) ;
    SendBTApayload(outbuf, datalen);
}
#endif

#if (EMMTRACELOG == 1 )
static void TraceAlloc(uint8_t tag, void *ptr, uint16_t len)
{
    OS_Use ( &tracelogSema ) ;

    if (tlAvailable == MX_TRACELOG)
    {
        OS_Unuse( &tracelogSema ); 
        wasfilled = true;
        ese_enqueue_once(ese005_05_tracelog_full);
        return;
    }

    tracelog[tlAvailable].tag = tag;
    tracelog[tlAvailable].ptr = ptr;
    tracelog[tlAvailable].len = len;
    tlAvailable++;
    allocCount++;
    OS_Unuse( &tracelogSema ); 
    if ( allocCount > allocCountHW )
        {
        allocCountHW=allocCount;
        allocCountLW= allocCount;
        }
    else if (allocCount < allocCountLW )
        {
        allocCountLW = allocCount ;
        }
    totMalloc += len ;
    if ( totMalloc > mxMalloc )
        {
        mxMalloc = totMalloc ;
        }
        
}


static void TraceFree(void *ptr)
{
    int i;
    
    OS_Use ( &tracelogSema ) ;
    
    for (i = 0; i < tlAvailable; i++)
    {
        if (tracelog[i].ptr == ptr)
        {
            totMalloc -= tracelog[i].len ;
            while ( i < tlAvailable - 1 )
            {
                tracelog[i] = tracelog[i + 1];
                i++;
            }
            // clear the last one
            memset ( &tracelog[tlAvailable - 1], 0, sizeof (TRACELOGITEM));
            tlAvailable--;
            allocCount--;
            OS_Unuse( &tracelogSema ); 
            return;
        }
    }
    // how is it possible NOT to find one (unless we were full before)
    OS_Unuse( &tracelogSema ); 
    if (wasfilled) return;
    ese_enqueue_once( ese029_1D_EMM_trace_table_miss ) ;
}
#endif


//---------------------------------------------------------------------------------------
// Wrapper around malloc() - mainly for memory testing

bool emm_init(void)
{
#if ( DELIBERATELY_FAIL_MALLOCS != 0 )
    srand(1234);
#endif
    
#if (EMMTRACELOG == 1 )
    OS_CREATERSEMA ( &tracelogSema ) ;
#endif
    
    return true;
}

int emmCount;

// cant do calloc until we add fences to it... todo 2
//#if ( EMMTRACELOG == 1 )
//void* emm_sys_calloc(uint8_t tag, uint16_t size)
//#else
//void* emm_sys_calloc(uint16_t size)
//#endif
//{
//    uint8_t *ptr = (uint8_t *)emm_dmalloc(tag, size);
//    if (ptr != NULL)
//    {
//        memset(ptr, 0, size);
//    }
//    return ptr;
//}


#if ( EMMTRACELOG == 1 )
void* emm_sys_malloc(uint8_t tag, uint16_t size)
#else
void* emm_sys_malloc(uint16_t size)
#endif
{
    uint8_t *ptr;

#if ( DELIBERATELY_FAIL_MALLOCS != 0 )
    static int startupPause = 1000;
    if (startupPause)
    {
        startupPause--;
    }
    else if ((rand() % DELIBERATELY_FAIL_MALLOCS ) == 0 )       // 1-in-X
    {
      // printf("\nFailing a malloc");
      SendBTAmessageF1("Deliberately failing a malloc: ", tag );
      uxFails++;
      return NULL;
    }
#endif
    
#if ( BAC_DEBUG == 1 )
    if ( size > 0x0C00 )
    {
      panic();
      return NULL;
    }
#else
#error
#endif

    emmCount++;

#if ( CHECK_FENCES == 1 )
    ptr = (uint8_t *)malloc(size + sizeof ( EMM_FENCE ) * 2);
#else
    ptr = (uint8_t *) malloc(size);
#endif

    
#if (EMMTRACELOG == 1 )
    if ( ptr == NULL ) {
      uxFails++;
      ese_enqueue ( ese004_04_failed_to_malloc ) ;
    }
    else
    {
      TraceAlloc(tag, ptr, size);
    }
#endif

#if ( CHECK_FENCES == 1 )
    if ( ptr == NULL ) return NULL ;
    ((EMM_FENCE *)ptr)->length = size ;
    ((EMM_FENCE *)ptr)->signature = 'E' ;
#if ( EMMTRACELOG == 1 )
    ((EMM_FENCE *)ptr)->tag = tag ;
#endif
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->length = size ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->signature = 'E' ;
#if ( EMMTRACELOG == 1 )
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->tag = tag ;
#endif
    return ptr + sizeof ( EMM_FENCE ) ;
#else
    return ptr;
#endif
}

#if ( EMMTRACELOG == 1 )
void* emm_sys_safe_calloc(uint8_t tag, uint16_t size)
#else
void* emm_sys_safe_calloc(uint16_t size)
#endif
{
    uint8_t *ptr;

#if ( BAC_DEBUG == 1 )
    if ( size > 0x0C00 )
    {
      panic();
      return NULL;
    }
#else
#error
#endif

    emmCount++;

#if ( CHECK_FENCES == 1 )
    // todo2 change this back to malloc, and TEST!
    ptr = (uint8_t *)calloc(size + sizeof ( EMM_FENCE ) * 2, 1);
#else
    ptr = (uint8_t *)calloc(size, 1);
#endif

    
#if (EMMTRACELOG == 1 )
    if ( ptr == NULL ) {
      uxFails++;
      ese_enqueue ( ese004_04_failed_to_malloc ) ;
    }
    else
    {
      TraceAlloc(tag, ptr, size);
    }
#endif

#if ( CHECK_FENCES == 1 )
    if ( ptr == NULL ) return NULL ;
    ((EMM_FENCE *)ptr)->length = size ;
    ((EMM_FENCE *)ptr)->signature = 'E' ;
#if ( EMMTRACELOG == 1 )
    ((EMM_FENCE *)ptr)->tag = tag ;
#endif
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->length = size ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->signature = 'E' ;
#if ( EMMTRACELOG == 1 )
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->tag = tag ;
#endif
    return ptr + sizeof ( EMM_FENCE ) ;
#else
    return ptr;
#endif
}


#if ( EMMTRACELOG == 1 )
void* emm_sys_safe_malloc(uint8_t tag, uint16_t size)
#else
void* emm_sys_safe_malloc(uint16_t size)
#endif
{
    uint8_t *ptr;

#if ( BAC_DEBUG == 1 )
    if (size > 0x0C00)
    {
        panic();
        return NULL;
    }
#else
#error
#endif

    emmCount++;

#if ( CHECK_FENCES == 1 )
    // todo2 change this back to malloc, and TEST!
    ptr = (uint8_t *)calloc(size + sizeof(EMM_FENCE) * 2, 1);
#else
    // todo2 change this back to malloc, and TEST!
    ptr = (uint8_t *)calloc(size, 1);
#endif


#if (EMMTRACELOG == 1 )
    if (ptr == NULL) {
        uxFails++;
        ese_enqueue(ese004_04_failed_to_malloc);
    }
    else
    {
        TraceAlloc(tag, ptr, size);
    }
#endif

#if ( CHECK_FENCES == 1 )
    if (ptr == NULL) return NULL;
    ((EMM_FENCE *)ptr)->length = size;
    ((EMM_FENCE *)ptr)->signature = 'E';
#if ( EMMTRACELOG == 1 )
    ((EMM_FENCE *)ptr)->tag = tag;
#endif
    ((EMM_FENCE *)(ptr + size + sizeof(EMM_FENCE)))->length = size;
    ((EMM_FENCE *)(ptr + size + sizeof(EMM_FENCE)))->signature = 'E';
#if ( EMMTRACELOG == 1 )
    ((EMM_FENCE *)(ptr + size + sizeof(EMM_FENCE)))->tag = tag;
#endif
    return ptr + sizeof(EMM_FENCE);
#else
    return ptr;
#endif
}

unsigned staticcount;

#define MEM_BLOCK_LIMIT 157

void emm_free(void *mptr)
{
#if ( CHECK_FENCES == 1 )
  uint8_t *ptr = (uint8_t *) mptr - sizeof ( EMM_FENCE ) ;
#endif
  
#if (EMMTRACELOG == 1 )
    TraceFree(ptr);
#endif
    
    emmCount--;
    
#if ( CHECK_FENCES == 1 )
    EMM_FENCE *fence = (EMM_FENCE *)ptr ; 
    if ( fence->signature != 'E' ) {
      // breakpoint here and take a look at the tag
      panic () ;
      return ;
    }
    uint16_t length = fence->length ;
    fence = (EMM_FENCE *)((uint8_t *) ptr + length + sizeof(EMM_FENCE)) ;
    if ( fence->signature != 'E'  ) {
      // breakpoint here and take a look at the tag
      panic () ;
      return ;
    }
    if ( fence->length != length  ) {
      // breakpoint here and take a look at the tag
      panic () ;
      return ;
    }
    free(ptr);

#else   // BAC_DEBUG
    
    free(mptr);
    // todo1 - RPM bug here, revealed by BTC (longer and longer,,, i think)
#endif  // BAC_DEBUG
}


