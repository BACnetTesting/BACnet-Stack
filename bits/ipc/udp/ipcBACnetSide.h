/**************************************************************************
*
* Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
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

#pragma once

#include "ipcCommon.h"

void ipc_callBackAnaUpdate(uint deviceID, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propId, float val);
void ipc_callBackBinUpdate(uint deviceID, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propId, bool val);
void ipc_callBackDeviceUpdate(uint deviceID, bool online);

bool ipc_init_BACnet(void);
void Create_Object(const uint devInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, const char *name, const char *description);



