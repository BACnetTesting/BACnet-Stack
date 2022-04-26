/**************************************************************************
*
* Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
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

#include "datalink.h"
#include "ai.h"
#include "ms-input.h"
#include "objectModel.h"
#include "emm.h"
#include "linklist.h"

void BACnet_Create_Object(
    const uint32_t					deviceInstance,
    const BACNET_OBJECT_TYPE		objectType,
    const uint32_t					objectInstance,
    const char 						*name,
    const char						*description,
    const BACNET_ENGINEERING_UNITS 	units)
{
#if 0
    // Find our virtual device
    VirtualDeviceInfo *vdev = Find_Virtual_Device(deviceInstance);
    if (vdev == NULL)
    {
        panic();
        return;
    }

    switch (objectType)
    {
    case OBJECT_ANALOG_INPUT:
        Analog_Input_Create(vdev->pDev, objectInstance, name, units, NULL);
        break;

    default:
        panic();
        break;

    }
#endif
}

#if 0
bool Analog_Input_Create(
    DEVICE_OBJECT_DATA *pDev,
    const uint32_t instance,
    const std::string& nameRoot,
    const BACNET_ENGINEERING_UNITS units,
    const UserObjectData *userObjectData
)
{
    std::string name = nameRoot;

    AnalogInputObject *newObject = new AnalogInputObject(
        //level,
        //lst,
        //devId,
        instance,
        units,
        name,
        name,
        userObjectData);

    pDev->analogInputs.push_back((BACnetObject *)newObject); // todo2 - why the cast?
    return true;
}


bool MultiState_Input_Create(
    DEVICE_OBJECT_DATA *pDev,
    const uint32_t instance,
    const char *name,
    const UserObjectData *userObjectData
)
{
    MultiStateInputObject *msi = (MultiStateInputObject *)emm_scalloc('M', sizeof(MultiStateInputObject));
    BACNET_CHARACTER_STRING *tname = characterstring_create_ansi(name);
    if (!emm_check_alloc_two(msi, tname))
    {
        panic();
        return false ;
    }

    msi->objectCommon.objectName = tname;
    msi->objectCommon.objectType = OBJECT_MULTI_STATE_INPUT;
    // msi->objectCommon.pDev = pDev;
    msi->objectCommon.userObjectData = (void *) userObjectData;

    LinkListAppend((void **) &pDev->multiStateInputs, msi);

    // std::string name = nameRoot;

    //AnalogInputObject *newObject = new AnalogInputObject(
    //    //level,
    //    //lst,
    //    //devId,
    //    instance,
    //    units,
    //    name,
    //    name,
    //    userObjectData);

    //pDev->analogInputs.push_back((BACnetObject *)newObject); // todo2 - why the cast?

    return true;
}
#endif