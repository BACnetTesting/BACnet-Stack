/**
 * @file
 * @author Steve Karg
 * @date October 2019
 * @brief Header file for a basic NPDU handler
 *
 * @section LICENSE
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

#ifndef NPDU_HANDLER_H
#define NPDU_HANDLER_H

#include "configProj.h"

#if (BITS_ROUTER_LAYER == 0 )

//#include <stddef.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include <stdint.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/bacdef.h"
//#include "bacnet/bacenum.h"
//#include "bacnet/apdu.h"
#include "bacnet/npdu.h"

typedef struct _BACNET_ROUTE BACNET_ROUTE;

#ifdef __cplusplus_disable
extern "C" {
#endif /* __cplusplus_disable */

    BACNET_STACK_EXPORT
    void npdu_handler2(
        BACNET_ROUTE *srcRoute,
        uint8_t *pdu,
        uint16_t pdu_len);
        
    BACNET_STACK_EXPORT
    void npdu_handler_cleanup(void);

    // 2020-08-16 EKH: Not used.
    //BACNET_STACK_EXPORT
    //void npdu_handler_init(
    //    uint16_t bip_net,
    //    uint16_t mstp_net);

    BACNET_STACK_EXPORT
    void npdu_router_handler(
        uint16_t snet,
        BACNET_GLOBAL_ADDRESS * src,
        uint8_t * pdu,
        uint16_t pdu_len);

    BACNET_STACK_EXPORT
    int npdu_router_send_pdu(
        uint16_t dnet,
        BACNET_GLOBAL_ADDRESS * dest,
        BACNET_NPCI_DATA * npci_data,
        uint8_t * pdu,
        unsigned int pdu_len);
    
    BACNET_STACK_EXPORT
    void npdu_router_get_my_address(
        uint16_t dnet,
        BACNET_GLOBAL_ADDRESS * my_address);

#ifdef __cplusplus_todo
}
#endif /* __cplusplus_disable */

#endif // #if (THIS_IS_NOT_A_ROUTER == 1 )
#endif
