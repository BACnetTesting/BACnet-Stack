/**************************************************************************
*
* Copyright (C) 2014-2016 ConnectEx, Inc.
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
*********************************************************************/

#include <stddef.h>

#ifdef _MSC_VER                 // Microsoft C

#define MSC_MALLOC      1       // (define for MSC leak test, undef for testing emm)
#define EMM_MEM_TEST    1

#if ( MSC_MALLOC == 1 )
#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <memory.h>
#include <stdlib.h>
#endif // MSC_MALLOC
#endif // _MSC_VER

#error
#include "rtos.h"
#include "CEDebug.h"
#include "debug.h"
#include "llist.h"
#include "emm.h"
#include "btaDebug.h"
#include "ese.h"

#ifdef _MSC_VER
#define OS_CREATERSEMA(a)
#define OS_Use(a)
#define OS_Unuse(a)
#endif

#if ( BAC_DEBUG == 1 )
__packed typedef struct {
  uint8_t   signature ;
  uint8_t   tag ;
  __packed uint16_t  length ;
} EMM_FENCE ;
#else
#error
#endif


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

#ifndef _MSC_VER
static OS_RSEMA tracelogSema ;
#endif

#ifdef _MSC_VER
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


void SendBTAmemoryStats(void)
{
#ifdef _MSC_VER
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

bool emm_init()
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

#if 0
#if ( EMMTRACELOG == 1 )
void* emm_sys_calloc(uint8_t tag, uint16_t size)
#else
void* emm_sys_calloc(uint16_t size)
#endif
{
    uint8_t *ptr = (uint8_t *)emm_dmalloc(tag, size);
    if (ptr != NULL)
    {
        memset(ptr, 0, size);
    }
    return ptr;
}
#endif

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

#if ( BAC_DEBUG == 1 )
#ifdef _MSC_VER
    // todo2 change this back to malloc, and TEST!
    ptr = calloc(size + sizeof ( EMM_FENCE ) * 2, 1);
#else
    ptr = OS_malloc(size+ sizeof ( EMM_FENCE ) * 2 );
#endif
#else
#ifdef _MSC_VER
    // todo2 change this back to malloc, and TEST!
    ptr = calloc(size 1);
#else
    ptr = OS_malloc(size );
#endif
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

#if ( BAC_DEBUG == 1 )
    if ( ptr == NULL ) return NULL ;
    ((EMM_FENCE *)ptr)->length = size ;
    ((EMM_FENCE *)ptr)->signature = 'E' ;
    ((EMM_FENCE *)ptr)->tag = tag ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->length = size ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->signature = 'E' ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->tag = tag ;
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
    if ( size > 0x0C00 )
    {
      panic();
      return NULL;
    }
#else
#error
#endif

    emmCount++;

#if ( BAC_DEBUG == 1 )
#ifdef _MSC_VER
    // todo2 change this back to malloc, and TEST!
    ptr = calloc(size + sizeof ( EMM_FENCE ) * 2, 1);
#else
    ptr = OS_malloc(size+ sizeof ( EMM_FENCE ) * 2 );
#endif
#else
#ifdef _MSC_VER
    // todo2 change this back to malloc, and TEST!
    ptr = calloc(size 1);
#else
    ptr = OS_malloc(size );
#endif
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

#if ( BAC_DEBUG == 1 )
    if ( ptr == NULL ) return NULL ;
    ((EMM_FENCE *)ptr)->length = size ;
    ((EMM_FENCE *)ptr)->signature = 'E' ;
    ((EMM_FENCE *)ptr)->tag = tag ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->length = size ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->signature = 'E' ;
    ((EMM_FENCE *)(ptr + size + sizeof (EMM_FENCE)))->tag = tag ;
    return ptr + sizeof ( EMM_FENCE ) ;
#else
    return ptr;
#endif
}

unsigned staticcount;

#define MEM_BLOCK_LIMIT 157

void emm_free(void *mptr)
{
#if ( BAC_DEBUG == 1 )
  uint8_t *ptr = (uint8_t *) mptr - sizeof ( EMM_FENCE ) ;
#endif
  
#if (EMMTRACELOG == 1 )
    TraceFree(ptr);
#endif
    
    emmCount--;
    
#if ( BAC_DEBUG == 1 )
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
    OS_free(ptr);
#else
    
#ifdef _MSC_VER
    free(mptr);
#else
    OS_free(mptr);
#endif
    
#endif
}


