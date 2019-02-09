/**
* @file
* @author Steve Karg
* @date 2016


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

#ifndef VMAC_H
#define VMAC_H

#include <stdint.h>
#include <stdbool.h>

/* define the max MAC as big as IPv6 + port number */
#define VMAC_MAC_MAX 18
/**
* VMAC data structure
*
* @{
*/
struct vmac_data {
    uint8_t mac[18];
    uint8_t mac_len;
};
/** @} */

    unsigned int VMAC_Count(void);
    struct vmac_data *VMAC_Find_By_Key(uint32_t device_id);
    bool VMAC_Find_By_Data(struct vmac_data *vmac, uint32_t *device_id);
    bool VMAC_Add(uint32_t device_id, struct vmac_data *pVMAC);
    bool VMAC_Delete(uint32_t device_id);
    bool VMAC_Different(
        struct vmac_data *vmac1,
        struct vmac_data *vmac2);
    bool VMAC_Match(
        struct vmac_data *vmac1,
        struct vmac_data *vmac2);
    void VMAC_Cleanup(void);
    void VMAC_Init(void);

#ifdef TEST
#include "ctest.h"
    void testVMAC(
        Test * pTest);
#endif

#endif
