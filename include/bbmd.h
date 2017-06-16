#pragma once 

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#include <windows.h>
#endif

#include <stdint.h>
// #include <inaddr.h>
#include "net.h"

#ifndef MAX_BBMD_ENTRIES
#define MAX_BBMD_ENTRIES 16
#endif

#ifndef MAX_FD_ENTRIES
#define MAX_FD_ENTRIES 16
#endif

/*Each device that registers as a foreign device shall be placed
in an entry in the BBMD's Foreign Device Table (FDT). Each
entry shall consist of the 6-octet B/IP address of the registrant;
the 2-octet Time-to-Live value supplied at the time of
registration; and a 2-octet value representing the number of
seconds remaining before the BBMD will purge the registrant's FDT
entry if no re-registration occurs. This value will be initialized
to the 2-octet Time-to-Live value supplied at the time of
registration.*/

typedef struct {
    bool valid;
    /* BACnet/IP address */
    struct in_addr dest_address;
    /* BACnet/IP port number - not always 47808=BAC0h */
    uint16_t dest_port;
    /* seconds for valid entry lifetime */
    uint16_t time_to_live;
    /* our counter */
    time_t seconds_remaining;   /* includes 30 second grace period */
} FD_TABLE_ENTRY;

typedef struct {
    /* true if valid entry - false if not */
    bool valid;
    /* BACnet/IP address */
    struct in_addr dest_address;        /* in network format */
    /* BACnet/IP port number - not always 47808=BAC0h */
    uint16_t dest_port;                 /* in network format */
    /* Broadcast Distribution Mask */
    struct in_addr broadcast_mask;      /* in network format */
} BBMD_TABLE_ENTRY;

