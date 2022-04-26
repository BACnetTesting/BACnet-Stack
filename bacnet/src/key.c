/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2003 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to 
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330 
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
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

/*#define TEST */
/*#define TEST_KEY */
#include "key.h"

/** @file key.c  Tests (only) of key encoding/decoding.  */

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

/* test the encode and decode macros */
void testKeys(
    Test * pTest)
{
    int type, id;
    int decoded_type, decoded_id;
    KEY key;

    for (type = 0; type < KEY_TYPE_MAX; type++) {
        for (id = 0; id < KEY_ID_MAX; id++) {
            key = KEY_ENCODE(type, id);
            decoded_type = KEY_DECODE_TYPE(key);
            decoded_id = KEY_DECODE_ID(key);
            ct_test(pTest, decoded_type == type);
            ct_test(pTest, decoded_id == id);
        }
    }

}

/* test the encode and decode macros */
void testKeySample(
    Test * pTest)
{
    int type, id;
    int type_list[] = { 0, 1, KEY_TYPE_MAX / 2, KEY_TYPE_MAX - 1, -1 };
    int id_list[] = { 0, 1, KEY_ID_MAX / 2, KEY_ID_MAX - 1, -1 };
    int type_index = 0;
    int id_index = 0;
    int decoded_type, decoded_id;
    KEY key;

    while (type_list[type_index] != -1) {
        while (id_list[id_index] != -1) {
            type = type_list[type_index];
            id = id_list[id_index];
            key = KEY_ENCODE(type, id);
            decoded_type = KEY_DECODE_TYPE(key);
            decoded_id = KEY_DECODE_ID(key);
            ct_test(pTest, decoded_type == type);
            ct_test(pTest, decoded_id == id);

            id_index++;
        }
        id_index = 0;
        type_index++;
    }

}

#ifdef TEST_KEY
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("key", NULL);
    /* add the individual tests */
/*  rc = ct_addTestFunction(pTest, testKeys); */
/*  assert(rc); */
    rc = ct_addTestFunction(pTest, testKeySample);
    assert(rc);
    /* run all the tests */
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    /* completed testing - cleanup */
    ct_destroy(pTest);

    return 0;
}
#endif /* LOCAL_TEST */
#endif
