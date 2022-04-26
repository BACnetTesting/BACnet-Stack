/**************************************************************************
*
* Copyright (C) 2009 Peter Mc Shane
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

#ifndef TRENDLOG_H
#define TRENDLOG_H

#include "configProj.h"

#if (BACNET_USE_OBJECT_TRENDLOG == 1 )

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "readrange.h"

#define TL_MAX_ENTRIES      100     /* Entries per datalog */


/* Error code for Trend Log storage */
typedef struct tl_error {
    uint16_t usClass;
    uint16_t usCode;
} TL_ERROR;

/* Bit string of up to 32 bits for Trend Log storage */

typedef struct tl_bits {
    uint8_t ucLen;  /* bytes used in upper nibble/bits free in lower nibble */
    uint8_t ucStore[4];
} TL_BITS;

/* Storage structure for Trend Log data
 *
 * Note. I've tried to minimise the storage requirements here
 * as the memory requirements for logging in embedded
 * implementations are frequently a big issue. For PC or
 * embedded Linux type setupz this may seem like overkill
 * but if you have limited memory and need to squeeze as much
 * logging capacity as possible every little byte counts!
 */

typedef struct tl_data_record {
    time_t tTimeStamp;      /* When the event occurred */
    uint8_t ucRecType;      /* What type of Event */
    uint8_t ucStatus;       /* Optional Status for read value in b0-b2, b7 = 1 if status is used */
    union {
        uint8_t ucLogStatus;        /* Change of log state flags */
        uint8_t ucBoolean;  /* Stored boolean value */
        float fReal;        /* Stored floating point value */
        uint32_t ulEnum;    /* Stored enumerated value - max 32 bits */
        uint32_t ulUValue;  /* Stored unsigned value - max 32 bits */
        int32_t lSValue;    /* Stored signed value - max 32 bits */
        TL_BITS Bits;       /* Stored bitstring - max 32 bits */
        TL_ERROR Error;     /* Two part error class/code combo */
        float fTime;        /* Interval value for change of time - seconds */
    } Datum;
} TL_DATA_REC;

#define TL_T_START_WILD 1       /* Start time is wild carded */
#define TL_T_STOP_WILD  2       /* Stop Time is wild carded */

#define MX_HANDLE_TL	10

/* Structure containing config and status info for a Trend Log */

typedef struct tl_log_info {
    BACNET_OBJECT   common;         // must be first
    bool bEnable;                   /* Trend log is active when this is true */
    BACNET_DATE_TIME StartTime;     /* BACnet format start time */
    time_t tStartTime;              /* Local time working copy of start time */
    BACNET_DATE_TIME StopTime;      /* BACnet format stop time */
    time_t tStopTime;               /* Local time working copy of stop time */
    uint8_t ucTimeFlags;            /* Shorthand info on times */
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE Source; /* Where the data comes from */
    uint32_t ulLogInterval;         /* Time between entries in seconds */
    bool bStopWhenFull;             /* Log halts when full if true */
    uint32_t ulRecordCount;         /* Count of items currently in the buffer */
    uint32_t ulTotalRecordCount;    /* Count of all items that have ever been inserted into the buffer */
    BACNET_LOGGING_TYPE LoggingType;        /* Polled/cov/triggered */
    bool bAlignIntervals;           /* If true align to the clock */
    uint32_t ulIntervalOffset;      /* Offset from start of period for taking reading in seconds */
    bool bTrigger;                  /* Set to 1 to cause a reading to be taken */
    int iIndex;                     /* Current insertion point */
    time_t tLastDataTime;

    char handle[MX_HANDLE_TL];		// rest API

    // todo 3 - You will be replacing this Logs array with dynamic rest interface.
    TL_DATA_REC Logs[TL_MAX_ENTRIES];
    
    uint iapDeviceHandle;
    char iap_dpQualifier[100];          // todo 3 - this is going to waste a lot of space, fix for scaling to 100 000 points 

} TL_LOG_INFO, TREND_LOG_DESCR ;

/*
 * Data types associated with a BACnet Log Record. We use these for managing the
 * log buffer but they are also the tag numbers to use when encoding/decoding
 * the log datum field.
 */

#define TL_TYPE_STATUS  0
#define TL_TYPE_BOOL    1
#define TL_TYPE_REAL    2
#define TL_TYPE_ENUM    3
#define TL_TYPE_UNSIGN  4
#define TL_TYPE_SIGN    5
#define TL_TYPE_BITS    6
#define TL_TYPE_NULL    7
#define TL_TYPE_ERROR   8
#define TL_TYPE_DELTA   9
#define TL_TYPE_ANY     10      /* We don't support this particular can of worms! */


void Trend_Log_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Trend_Log_Valid_Instance(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance);

unsigned Trend_Log_Count(
    DEVICE_OBJECT_DATA* pDev);

uint32_t Trend_Log_Index_To_Instance(
    DEVICE_OBJECT_DATA *pDev,
    unsigned index);

bool Trend_Log_Object_Name(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

int Trend_Log_Read_Property(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Trend_Log_Write_Property(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_WRITE_PROPERTY_DATA * wp_data);

BACNET_OBJECT* Trend_Log_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance,
    const char* name,
    const uint iap_DeviceHandle,
    const char * iap_DPqualifier) ;

void Trend_Log_Init(
        void);

void TL_Insert_Status_Rec(
    TREND_LOG_DESCR *currentObject,
    BACNET_LOG_STATUS eStatus,
    bool bState);

bool TL_Is_Enabled(
    TREND_LOG_DESCR* currentObject);

time_t TL_BAC_Time_To_Local(
    BACNET_DATE_TIME * SourceTime);

void TL_Local_Time_To_BAC(
    BACNET_DATE_TIME * DestTime,
    time_t SourceTime);

int TL_encode_entry(
    TREND_LOG_DESCR* currentObject,
    uint8_t * apdu,
    int iLog,
    int iEntry);

int TL_encode_by_position(
	DEVICE_OBJECT_DATA* pDev,
	TREND_LOG_DESCR* currentObject,
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest);

int TL_encode_by_sequence(
	DEVICE_OBJECT_DATA* pDev,
	TREND_LOG_DESCR* currentObject,
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest);

int TL_encode_by_time(
	DEVICE_OBJECT_DATA* pDev,
	TREND_LOG_DESCR* currentObject,
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest);

bool TrendLogGetRRInfo(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_RANGE_DATA * pRequest,      /* Info on the request */
    RR_PROP_INFO * pInfo);                  /* Where to put the information */

int rr_trend_log_encode(
    DEVICE_OBJECT_DATA* pDev,
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest);

void trend_log_timer(
    DEVICE_OBJECT_DATA * pDev,
    uint16_t uSeconds);

#endif  // ( BACNET_USE_OBJECT_XXX == 1 )   -   configProj.h includes this BACnet Object Type
#endif  // XXX_H include guard
