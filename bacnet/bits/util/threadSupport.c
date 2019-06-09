/**************************************************************************

    Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.

    This program is free software : you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.If not, see <http://www.gnu.org/licenses/>.

    For more information : info@bac-test.com
        For access to source code : info@bac-test.com
            or www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

/*

    Of threads, queues and sockets.

    Smarter people than me (*1) have said that a Berkeley socket should be treated as a shared resource
    and BACnet has to use the same socket for receiving as well as sending. Thus this socket either
    has to be protected by a locking mechanism of some sort, else the socket should only be used in
    a single thread.

    And we SHOULD NOT recv() on a socket and elsewhere use it to send() !

    Furthermore, other smart people have stated that a queue mechanism is MUCH higher performance
    than mutexs, critical sections, etc...

    And of course, in smaller processors, if they don't use threads, probably implement the IO using
    interrupts, which make the above additionally relevant.

    Also, with interfaces hot-plugging, going down, coming up, retrying (think Lon, USB, WiFi) 
    the adapter state may be ever changing, requiring at least a little buffering before sending.

    Too, splitting datalinks across processors (think Co-Processors) adds a disconnect.

    The disconnect between high speed (multiple gigabit ethernet to multiple RS485 MSTP) adds
    a queue monitoring and management issue.

    So rather than avoiding queues between datalink and app, I have chosen to embrace them. Fully. So be it.

    Ed.

*/

