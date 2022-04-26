
        Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

        This program is free software; you can redistribute it and/or
        modify it under the terms of the GNU General Public License
        as published by the Free Software Foundation; either version 2
        of the License, or (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to:
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


The IPC layer serves as a communications link between an Application and the BACnet stack
=========================================================================================

It may be implemented serially, via UDP sockets, shared memory etc.

The intent is for the application to not to worry about "BACnet Stuff", but yet still be able to

        a) Create BACnet Devices, Objects
        b) Populate their BACnet Properties
        c) Update them with real-time values

Everything is Big Endian
Because of different architectures and word sizes, all ints are defined explicitly
Floats are IEEE format

The 'commands' necessary for this link may be implemented in various ways, but as a starting point,
they need values..

        0b00xxxxxx      Application to BACnet messages
        0b10xxxxxx      BACnet to App messages
        0b11xxxxxx      Bidirectional


Bi-directional (hex)
====================

A0      Ack
E0      Error - Command not implemented

Application to BACnet (hex)
===========================

Code    Command                 Fields/(comments)
----    -------                 -----------------
01      Restart BACnet
02      Ping                    (used to determine if App is alive)
03      Ping response

20      Set MSTP baud rate      U8 datalinkID, U32 baud
21      Set MSTP MAC addr       U8 datalinkID, U8 addr
22      Set MSTP Max Masters    U8 datalinkID, U8 maxMasters
23      Set MSTP Max Info       U8 datalinkID, U8 maxInfo

30      Set IP address          U8 datalinkID, U32 ipAddr
31      Set IP netmask          U8 datalinkID, U32 netMask
32      Set IP gateway          U8 datalinkID, U32 gateway
33      Set BACnet Port         U8 datalinkID, U16 port

50      Create Device Object    U32 devInstance, STRING deviceName
51      Create Object           U32 devInstance, OBJID objID, STRING objectName
52      Create Property         U32 devInstance, OBJID objID, U16 property

60      Update Property         U32 devInstance, OBJID objID, U16 property, VALUE value
61      Update PV               U32 devInstance, OBJID objID, VALUE value                       (short version of 0x60, 'property' not required)

7f      Command extension       TBD if required


BACnet to Application (hex)
===========================

Code    Command                 Fields/(comments)
----    -------                 -----------------
81      Restart App
82      Ping                    (used to determine if interface is alive)
83      Ping response

91      Update PV real          U32 devInstance, OBJID objID, VALUE value



Datatypes: STRING, OBJID, FLOAT
-------------------------------

The data type and format is defined by a tagged system based on the BACnet specification. This means the type and the length
of the data field is embedded in the parameter. See the specification, "20.2.1 General Rules For Encoding BACnet Tags"

Since the data format is 'self referencing' fields that specify VALUE could be in any of the above formats.



