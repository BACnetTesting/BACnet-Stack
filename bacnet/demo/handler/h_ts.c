/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

#include <stddef.h>
#include <time.h>
#include "device.h"
#include "datetime.h"
//#include "bacdef.h"
#include "timesync.h"
#include "client.h"
#include "debug.h"
#include "bitsDebug.h"

/** @file h_ts.c  Handles TimeSync requests. */

#if (BACNET_TIME_MASTER == 1)
/* sending time sync to recipients */
#ifndef MAX_TIME_SYNC_RECIPIENTS
#define MAX_TIME_SYNC_RECIPIENTS 16
#endif

BACNET_RECIPIENT_LIST Time_Sync_Recipients[MAX_TIME_SYNC_RECIPIENTS];
/* variable used for controlling when to
   automatically send a TimeSynchronization request */
static BACNET_DATE_TIME Next_Sync_Time;
#endif

static BACNET_DATE_TIME bdt;

static void show_bacnet_date_time(
    BACNET_DATE * bdate,
    BACNET_TIME * btime)
{
    /* show the date received */
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "%u", (unsigned) bdate->year);
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "/%u", (unsigned) bdate->month);
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "/%u", (unsigned) bdate->day);
    /* show the time received */
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, " %02u", (unsigned) btime->hour);
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, ":%02u", (unsigned) btime->min);
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, ":%02u", (unsigned) btime->sec);
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, ".%02u", (unsigned) btime->hundredths);
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "\r\n");
}

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
extern int32_t emulationOffset;
#endif

void handler_timesync(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE *rxDetails )
{
    int len ;
//    BACNET_DATE bdate;
//    BACNET_TIME btime;

    (void) service_len;
    (void) rxDetails;

    len =
        timesync_decode_service_request(service_request, service_len, &bdt.date, &bdt.time);
    if (len > 0) {
        if (datetime_is_valid(&bdt.date, &bdt.time)) {
            /* fixme: only set the time if off by some amount */
        dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "Received TimeSyncronization Request\r\n");
        show_bacnet_date_time(&bdt.date, &bdt.time);

#if defined ( _MSC_VER  )
        struct tm *tLocal ;
        struct tm tSet = { 0 } ;
        time_t clockTime;
        time_t localTime;
        time_t setTime;
        /*
        struct tm

        int    tm_sec   Seconds [0,60].
        int    tm_min   Minutes [0,59].
        int    tm_hour  Hour [0,23].
        int    tm_mday  Day of month [1,31].
        int    tm_mon   Month of year [0,11].
        int    tm_year  Years since 1900.
        int    tm_wday  Day of week [0,6] (Sunday =0).
        int    tm_yday  Day of year [0,365].
        int    tm_isdst Daylight Savings flag.
        */

        time(&clockTime);
        tLocal = (struct tm *) localtime(&clockTime);
        localTime = mktime(tLocal);

        tSet.tm_sec = bdt.time.sec;
        tSet.tm_min = bdt.time.min;
        tSet.tm_hour = bdt.time.hour;
        tSet.tm_mday = bdt.date.day;
        tSet.tm_mon = bdt.date.month - 1 ;
        tSet.tm_year = bdt.date.year - 1900 ;
        tSet.tm_isdst = -1; // tLocal->tm_isdst;    // -1 means 'figure it out'

        setTime = mktime(&tSet);

        if (setTime > 0)
        {
            time_t delta = setTime - localTime;
            emulationOffset = delta ;
        }
#else
        struct tm tSet = { 0 } ;
        time_t clockTime;
        time_t localTime;
        time_t setTime;
        /*
        struct tm
        }
        int    tm_sec   Seconds [0,60].
            /* FIXME: set the time?
        int    tm_hour  Hour [0,23].
        int    tm_mday  Day of month [1,31].
        int    tm_mon   Month of year [0,11].
        int    tm_year  Years since 1900.
        int    tm_wday  Day of week [0,6] (Sunday =0).
        int    tm_yday  Day of year [0,365].
        int    tm_isdst Daylight Savings flag.
        */

        time(&clockTime);
        tLocal = (struct tm *) localtime(&clockTime);
        localTime = mktime(tLocal);
#endif 
        tSet.tm_sec = bdt.time.sec;
        tSet.tm_min = bdt.time.min;
        tSet.tm_hour = bdt.time.hour;
        tSet.tm_mday = bdt.date.day;
        tSet.tm_mon = bdt.date.month - 1 ;
        tSet.tm_year = bdt.date.year - 1900 ;
        tSet.tm_isdst = -1; // tLocal->tm_isdst;    // -1 means 'figure it out'
#if ! defined ( _MSC_VER )
        setTime = mktime(&tSet);
        if (setTime > 0)
        {
            time_t delta = setTime - localTime;
            emulationOffset = delta ;
        }
#endif
        }
    }
}


void handler_timesync_utc(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE *rxDetails)
{
    (void) service_len;
    (void) rxDetails;
    int len =
        timesync_decode_service_request(service_request, service_len, &bdt.date,
        &bdt.time);
    if (len > 0) {
        if (datetime_is_valid(&bdt.date, &bdt.time)) {
            dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "Received TimeSyncronization Request\r\n");
            show_bacnet_date_time(&bdt.date, &bdt.time);
        }
    }
            /* FIXME: set the time?
               only set the time if off by some amount */
}


#if (BACNET_TIME_MASTER == 1)
/** Handle a request to list all the timesync recipients.
 *
 *  Invoked by a request to read the Device object's
 *  PROP_TIME_SYNCHRONIZATION_RECIPIENTS.
 *  Loops through the list of timesync recipients, and, for each valid one,
 *  adds its data to the APDU.
 *
 *  @param apdu [out] Buffer in which the APDU contents are built.
 *  @param max_apdu [in] Max length of the APDU buffer.
 *
 *  @return How many bytes were encoded in the buffer, or
 *   BACNET_STATUS_ABORT if the response would not fit within the buffer.
 */
int handler_timesync_encode_recipients(
    uint8_t * apdu,
    int max_apdu)
{
    return timesync_encode_timesync_recipients(apdu, max_apdu,
        &Time_Sync_Recipients[0]);
}
#endif


#if (BACNET_TIME_MASTER == 1)
bool handler_timesync_recipient_write(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;

    /* fixme: handle writing of the recipient list */
    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;

    return status;
}
#endif


#if (BACNET_TIME_MASTER == 1)
static void handler_timesync_send(
    BACNET_DATE_TIME * current_date_time)
{
    unsigned index = 0;
    bool status = false;

    for (index = 0; index < MAX_TIME_SYNC_RECIPIENTS; index++) {
        if (Time_Sync_Recipients[index].tag == 1) {
            if (status) {
                Send_TimeSync_Remote(
                    &Time_Sync_Recipients[index].type.address,
                    &current_date_time->date,
                    &current_date_time->time);
            }
        }
    }
}
#endif


#if (BACNET_TIME_MASTER == 1)
static void handler_timesync_update(
    uint32_t device_interval,
    BACNET_DATE_TIME * current_date_time)
{
    uint32_t current_minutes = 0;
    uint32_t next_minutes = 0;
    uint32_t delta_minutes = 0;
    uint32_t offset_minutes = 0;
    uint32_t interval = 0;
    uint32_t interval_offset = 0;

    datetime_copy(&Next_Sync_Time, current_date_time);
    if (Device_Align_Intervals()) {
        interval_offset = Device_Interval_Offset();
        /* If periodic time synchronization is enabled and
           the time synchronization interval is a factor of
           (divides without remainder) an hour or day, then
           the beginning of the period specified for time
           synchronization shall be aligned to the hour or
           day, respectively. */
        if ((60 % device_interval) == 0) {
            /* factor of an hour alignment */
            /* Interval_Minutes = 1  2  3  4  5  6  10  12  15  20  30  60 */
            /* determine next interval */
            current_minutes = Next_Sync_Time.time.min;
            interval = current_minutes/device_interval;
            interval++;
            next_minutes = interval * device_interval;
            offset_minutes = interval_offset % device_interval;
            next_minutes += offset_minutes;
            delta_minutes = next_minutes - current_minutes;
            datetime_add_minutes(&Next_Sync_Time, delta_minutes);
            Next_Sync_Time.time.sec = 0;
            Next_Sync_Time.time.hundredths = 0;
        } else if ((1440 % device_interval) == 0) {
            /* factor of a day alignment */
            /* Interval_Minutes = 1  2  3  4  5  6  8  9  10  12  15  16
               18  20  24  30  32  36  40  45  48  60  72  80  90  96  120
               144  160  180  240  288  360  480  720  1440   */
            current_minutes =
                datetime_minutes_since_midnight(&Next_Sync_Time.time);
            interval = current_minutes/device_interval;
            interval++;
            next_minutes = interval * device_interval;
            offset_minutes = interval_offset % device_interval;
            next_minutes += offset_minutes;
            delta_minutes = next_minutes - current_minutes;
            datetime_add_minutes(&Next_Sync_Time, delta_minutes);
            Next_Sync_Time.time.sec = 0;
            Next_Sync_Time.time.hundredths = 0;
        }
    } else {
        datetime_add_minutes(&Next_Sync_Time, device_interval);
        Next_Sync_Time.time.sec = 0;
        Next_Sync_Time.time.hundredths = 0;
    }
}
#endif


#if (BACNET_TIME_MASTER == 1)
bool handler_timesync_recipient_address_set(
    unsigned index,
    BACNET_PATH *address)
{
    bool status = false;

    if (address && (index < MAX_TIME_SYNC_RECIPIENTS)) {
        Time_Sync_Recipients[index].tag = 1;
        bacnet_address_copy(
            &Time_Sync_Recipients[index].type.address,
            address);
        status = true;
    }

    return status;
}
#endif


#if (BACNET_TIME_MASTER == 1)
void handler_timesync_task(
    BACNET_DATE_TIME * current_date_time)
{
    int compare = 0;
    uint32_t device_interval = 0;

    device_interval = Device_Time_Sync_Interval();
    if (device_interval) {
        compare = datetime_compare(current_date_time, &Next_Sync_Time);
       /* if the date/times are the same, return is 0
          if date1 is before date2, returns negative
          if date1 is after date2, returns positive */
        if (compare >= 0) {
            handler_timesync_update(device_interval, current_date_time);
            handler_timesync_send(current_date_time);
        }
    }
}
#endif


#if (BACNET_TIME_MASTER == 1)
void handler_timesync_init(void)
{
    unsigned i = 0;

    /* connect linked list */
    for (; i < (MAX_TIME_SYNC_RECIPIENTS-1); i++) {
        Time_Sync_Recipients[i].next = &Time_Sync_Recipients[i+1];
        Time_Sync_Recipients[i+1].next = NULL;
    }
    for (i = 0; i < MAX_TIME_SYNC_RECIPIENTS; i++) {
        Time_Sync_Recipients[i].tag = 0xFF;
    }
}
#endif
