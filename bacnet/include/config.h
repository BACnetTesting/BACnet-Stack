/**************************************************************************
*
* Copyright (C) 2004 Steve Karg <skarg@users.sourceforge.net>
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

/*
    EKH: 
    Using configProj.h, that is in the same directory as other include files, is problematic when:
    1. Obeying the rule that config files should 'compile' on their own (*1), and include other config files to make it so,
    2. Trying to have multiple projects in a source tree, each with their own config file to set their unique profile.
    Item 1 will result in this config file being included EVEN if the intent is to use a project config file, selected 
    in preference by the order of include paths.
    To eliminate this problem, I created configProj.h for project specific configurations. And that file must not
    ever reside in this directory ( bacnet/include )

    *1  http://umich.edu/~eecs381/handouts/CHeaderFileGuidelines.pdf
*/

#ifndef CONFIG_H
#define CONFIG_H

// redirecting to configProject.h (or any other) allows compile time switching between projects

#ifndef CTCONFIG
#include "configProj.h"
// #include "..\app\yourCo\yourProj\configProj.h"
#else
#include CTCONFIG
#endif

#endif // CONFIG_H

