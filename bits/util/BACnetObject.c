/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

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

#include <stdint.h>
#include "bacstr.h"
#include "BACnetObject.h"
#include "debug.h"
#include "bitsDebug.h"
#include "llist.h"

BACNET_OBJECT *Generic_Instance_To_Object(
    LLIST_HDR *objectHdr,
    const uint32_t objectInstance)
{
    BACNET_OBJECT *bacnetObject = (BACNET_OBJECT *) objectHdr->first;
    while (bacnetObject != NULL) {
        if (bacnetObject->objectInstance == objectInstance) return bacnetObject;
        bacnetObject = (BACNET_OBJECT *)bacnetObject->llist.next;
    }
    dbTraffic(DBD_ALL, DB_BTC_ERROR, "Illegal Instance, %d", objectInstance );
    return bacnetObject;
}


bool Generic_Instance_To_Object_Name(
    LLIST_HDR *objectHdr,
    uint32_t objectInstance,
    BACNET_CHARACTER_STRING *object_name)
{
    BACNET_OBJECT *bacnetObject = Generic_Instance_To_Object(objectHdr, objectInstance);
    if (bacnetObject == NULL) return false;
    
    return characterstring_copy(object_name, &bacnetObject->objectName);
}


void Generic_Object_Init(
    BACNET_OBJECT       *bacnetObject,
    const   uint32_t    objectInstance,
    const   char        *objectName)
{
    bacnetObject->objectInstance = objectInstance;
    characterstring_init_ansi(&bacnetObject->objectName, objectName);
}


uint32_t Generic_Index_To_Instance(
    LLIST_HDR *objectHdr,
    uint32_t objectIndex )
{
    unsigned count = 0;
    BACNET_OBJECT *bacnetObject = (BACNET_OBJECT *)objectHdr->first;
    while (bacnetObject != NULL) {
        if (count == objectIndex) return bacnetObject->objectInstance;
        count++;
        bacnetObject = (BACNET_OBJECT *)bacnetObject->llist.next;
    }
    panic();
    return count;
}


BACNET_OBJECT *Generic_Index_To_Object(
    LLIST_HDR *objectHdr,
    const uint32_t objectIndex)
{
    return ((BACNET_OBJECT *)ll_GetPtr(objectHdr, objectIndex));
}


