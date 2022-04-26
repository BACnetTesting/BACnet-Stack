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

#ifndef NET_H
#define NET_H

#ifndef IPPROTO_IPV6
   // If the version of winsock does not by default include IPV6 then
   // use the tech preview if it is avaliable.

    // 2018.05.07 - EKH: Can someone explain where to get this file?
    // Is it even still required?
    // #include <tpipv6.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <sys/timeb.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>

#include <linux/if_link.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>



#if 0

// close conflicts with file close in Microsoft MSVC
#define close closesocket

typedef int socklen_t;

#endif
#endif
