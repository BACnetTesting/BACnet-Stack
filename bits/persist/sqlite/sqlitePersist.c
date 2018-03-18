/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"

#include "sqlite3.h"
#include "bitsPersist.h"
#include "bitsUtil.h"
#include "BACnetToString.h"
#include "device.h"

extern PORT_SUPPORT *applicationDatalink;
extern DEVICE_OBJECT_DATA applicationDevice;

// sqlite help: https://sqlite.org/c3ref/bind_blob.html

static const char *persistDbName = "BACnetPersistData.sqlite";
static const char *routerportConfigTableName = "RouterPorts";

static sqlite3 *db;

static int OpenDatabase(void)
{
    int rc = sqlite3_open_v2(persistDbName, &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        log_lev_printf(LLEV::Release, "Can't open persistence database %s, autocreating a new one: \"%s\"", persistDbName, sqlite3_errmsg(db));
        int rc = sqlite3_open_v2(persistDbName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        if (rc != SQLITE_OK)
        {
            log_lev_printf(LLEV::Release, "Can't create persistence database %s: \"%s\"", persistDbName, sqlite3_errmsg(db));
            return false;
        }
    }
    return rc;
}


static const char *rpcColNameAdapter = "Adapter";
static const char *rpcColNameBACnetPort = "Port";
static const char *rpcColNamePortType = "PortType";
static const char *rpcColNameNN = "NetworkNumber" ;
static const char *rpcColNameHomeDeviceID = "HomeDeviceID";
static const char *rpcColNameEndPoint = "EndPoint";


static int32_t RouterPortCallback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    char *adapter = NULL ;
    char *portType = NULL;
    char *endpoint = NULL;
    char *homeDeviceIDstr = NULL;
    int networkNumber ;
    int bacnetPort ;
    bool ok;
    SOCKADDR_IN ipep;

    // scan the column names, picking up the values
    for (i = 0; i < argc; i++)
    {
        if ( isMatchCaseInsensitive(azColName[i], rpcColNameAdapter) )
        {
            adapter = argv[i];
        }
        else if (isMatchCaseInsensitive(azColName[i], rpcColNameNN))
        {
            networkNumber = atoi(argv[i]);
        }
        else if (isMatchCaseInsensitive(azColName[i], rpcColNamePortType))
        {
            portType = argv[i] ;
        }
        else if (isMatchCaseInsensitive(azColName[i], rpcColNameEndPoint))
        {
            endpoint = argv[i];
        }
        else if (isMatchCaseInsensitive(azColName[i], rpcColNameHomeDeviceID)) // 107
        {
            homeDeviceIDstr = argv[i];
        }
        else if (isMatchCaseInsensitive(azColName[i], rpcColNameBACnetPort))
        {
            bacnetPort = atoi(argv[i]);
        }
        else
        {
            panic();
            return 1;
        }
    }

    switch ( StringTo_PF(portType))
    {
    case PF_BIP:
        log_printf("BACnet Port          %d, Network Number %d", bacnetPort, networkNumber);
        ok = InitRouterport(PF_BIP, adapter, (uint16_t) networkNumber, (uint16_t) bacnetPort );
		if (!ok) {
			panic();
			return 1;
		}
        break; 

    case PF_BBMD:
        log_printf("BACnet BBMD Port     %d, Network Number %d", bacnetPort, networkNumber);
        ok = InitRouterport(PF_BBMD, adapter, (uint16_t)networkNumber, (uint16_t)bacnetPort);
		if (!ok) {
			panic();
			return 1;
		}
        break;

    case PF_NAT:
        log_printf("BACnet NAT Port      %d, Network Number %d, EndPoint %s", bacnetPort, networkNumber, endpoint );
        if (!StringTo_IPEP(&ipep, endpoint))
        {
            panic();
            return 1;
        }
        ok = InitRouterportWithNAT(PF_NAT, adapter, (uint16_t)networkNumber, (uint16_t)bacnetPort, &ipep );
        if (!ok) {
            panic();
            return 1;
        }
        break;

    case PF_VIRT:
        log_printf("Virtual Network             Network Number %d", networkNumber);
        ok = InitRouterportVirtual(networkNumber);
		if (!ok) {
			panic();
			return 1;
		}
        break;

    case PF_APP:
        ok = InitRouterportApp(networkNumber);          // Network Number to 'associate' with.
		if (!ok) {
			panic();
			return 1;
		}
        break;

    case PF_MSTP:
    case PF_FD:
    	panic();
    	return 1 ;
    	break;

    }

    if ( homeDeviceIDstr != NULL )
    {
    	int devid = atoi(homeDeviceIDstr);

    	// this is our application entity router port
        ok = InitRouterportApp(networkNumber);          // Network Number to 'associate' with.
		if (!ok) {
			panic();
			return 1;
		}

	    Device_Init_New( PF_APP, applicationDatalink, &applicationDevice, devid, "Virtual Router Device", "BACnet Virtual Router", NULL );
    }

    return 0;
}


void LoadRouterPortConfigs(void)
{
    int rc;
    char command[1000];
    char *zErrMsg = 0;

    OpenDatabase();

    // Do an *attempt* to create the table (It should fail, cos table already there, if not, then proceed to populate defaults)
    sprintf(command, "create table %s ( %s STRING, %s INTEGER NOT NULL, %s STRING, %s INTEGER NOT NULL, %s INTEGER, %s STRING)",
        routerportConfigTableName,
        rpcColNameAdapter,
        rpcColNameBACnetPort,
        rpcColNamePortType,
        rpcColNameNN,
		rpcColNameHomeDeviceID,
        rpcColNameEndPoint );
    rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
    if (rc == SQLITE_OK) {

        // means that the table did NOT exist, and we have just successfully created a new one, so enter a factory default.
        // Note that we do not create a NAT routerport - that can be achieved via sqlite3: INSERT INTO RouterPorts VALUES .. 
        sprintf(command, "insert into %s (%s, %s, %s, %s, %s) values ( '%s', %d, '%s', %d, %d)",
            routerportConfigTableName,
            rpcColNameAdapter,
            rpcColNameBACnetPort,
            rpcColNamePortType,
            rpcColNameNN,
            rpcColNameHomeDeviceID,
            "eth0",
            47808,
            PF_ToString(PF_BIP),
            1111,
			3448473 );
        rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            panic();
        }
#if 0 // enough for now, use sqlite if you need more ports
        sprintf(command, "insert into %s (%s, %s, %s, %s) values ( '%s', %d, '%s', %d)",
            routerportConfigTableName, 
            rpcColNameAdapter,
            rpcColNameBACnetPort,
            rpcColNamePortType,
            rpcColNameNN,
            "eth0",
            47809,
            PF_ToString(PF_BBMD),
            1112);
        rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            panic();
        }
        sprintf(command, "insert into %s (%s, %s, %s, %s) values ( '%s', %d, '%s', %d)",
            routerportConfigTableName,
            rpcColNameAdapter,
            rpcColNameBACnetPort,
            rpcColNamePortType,
            rpcColNameNN,
            "",
            0,
            PF_ToString(PF_VIRT),
            1113);
        rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            panic();
        }
        sprintf(command, "insert into %s (%s, %s, %s, %s) values ( '%s', %d, '%s', %d)",
            routerportConfigTableName,
            rpcColNameAdapter,
            rpcColNameBACnetPort,
            rpcColNamePortType,
            rpcColNameNN,
            "",
            0,
            PF_ToString(PF_APP),
            1111);
        rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            panic();
        }
#endif
    }

    // Get the entries (or the recently added factory default) using callback method
    sprintf(command, "select * from %s", routerportConfigTableName);
    rc = sqlite3_exec(db, command, RouterPortCallback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        panic();
        // don't return, we still want to close
    }

    sqlite3_close(db);

    // must be done at the end since we don't know what order the ports are going to be created in.
    AlignApplicationWithPort();
}


// Persistence for Notification Class (only required for e.g. Intrinsic Alarming)
#if 0
static char *recipientTableName = "Recipients";
static char *colNameDeviceInst = "DeviceInst";
static char *colNameNCinst = "NCinst";
static char *colNameRecipientList = "Recipient";
static char *colNameRecipientListLen = "RecipientLen";

static sqlite3 *db;

bool PersistRecipientOpenWrite(void)
{
    int rc;
    char *zErrMsg = 0;
    char command[1000];

    rc = sqlite3_open_v2(persistDbName, &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        // database does not exist, create it
        rc = sqlite3_open_v2(persistDbName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        if (rc != SQLITE_OK)
        {
            log_lev_printf(LLEV::Release, "Can't create database %s for Device Object Mappings database: \"%s\"", persistDbName, sqlite3_errmsg(db));
            return false;
        }
        // we have just created an empty database, now create the table we need
    }
    else
    {
        // Database already existed, so drop the old Recipients table
        sprintf(command, "DROP TABLE[%s]", recipientTableName);
        rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            log_lev_printf(LLEV::Release, "Can't drop table [%s], error \"%s\"", recipientTableName, sqlite3_errmsg(db));
            // no worries, tables may have changed, proceed
            // sqlite3_close(db);
            // return false ;
        }
    }

    sprintf(command, "CREATE TABLE[%s](%s INTEGER NOT NULL, %s INTEGER NOT NULL, %s BLOB, %s INTEGER NOT NULL)", recipientTableName, colNameDeviceInst, colNameNCinst, colNameRecipientList, colNameRecipientListLen);
    rc = sqlite3_exec(db, command, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        log_lev_printf(LLEV::Release, "Can't create table [%s], error \"%s\"", recipientTableName, sqlite3_errmsg(db));
        // no worries, tables may have changed, proceed
        // sqlite3_close(db);
        // return false ;
    }

    return true;
}

bool PersistRecipientOpenRead(void)
{
    int rc;
    char *zErrMsg = 0;
    char command[1000];

    rc = sqlite3_open_v2(persistDbName, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }
    return true;
}


void PersistRecipientClose(void)
{
    sqlite3_close(db);
}


void PersistRecipientAdd(int devInst, int ncInst, void *recipientList, int len)
{
    // can we open and process another table in the meantime??
    int rc;
    char *zErrMsg = 0;
    char command[1000];


    // prepare the insert statement
    sprintf(command, "insert into %s (%s, %s, %s, %s) values ( %d, %d, ?, %d )", recipientTableName, colNameDeviceInst, colNameNCinst, colNameRecipientList, colNameRecipientListLen, devInst, ncInst, len);
    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2(db, command, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        log_lev_printf(LLEV::Release, "SQL error [%s] preparing insert statement", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    //rc = sqlite3_bind_int(stmt, 1, devInst );
    //if (rc != SQLITE_OK) {
    //    log_lev_printf(LLEV::Release, "SQL error preparing bind");
    //    sqlite3_close(db);
    //    return;
    //}
    //rc = sqlite3_bind_int(stmt, 2, ncInst );
    //if (rc != SQLITE_OK) {
    //    log_lev_printf(LLEV::Release, "SQL error preparing bind");
    //    sqlite3_close(db);
    //    return;
    //}
    rc = sqlite3_bind_blob(stmt, 1, recipientList, len, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        log_lev_printf(LLEV::Release, "SQL error preparing bind");
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        log_lev_printf(LLEV::Release, "SQL error [%s] in step", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        log_lev_printf(LLEV::Release, "SQL error in finalize");
        sqlite3_close(db);
        return;
    }

}


void PersistRecipientLists(void)
{
    printf("Persisting all recipient lists...\n");


    // Open the database
    if (!PersistRecipientOpenWrite())
    {
        panic();
        return;
    }

    // Note, this gets ALL objects for ALL virtual devices!
    PersistRecipientClose();
}





static int32_t RecipientListCallback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    char *name = "Unassigned";
    char *desc = "No Description";

    int deviceInst;
    int ncInst;
    int recipientLen;
    void *recipient = NULL;

    // scan the column names, picking up the values
    for (i = 0; i < argc; i++)
    {
        if (_strcmpi(azColName[i], colNameDeviceInst) == 0)
        {
            deviceInst = atoi(argv[i]);
        }
        else if (_strcmpi(azColName[i], colNameNCinst) == 0)
        {
            ncInst = atoi(argv[i]);
        }
        else if (_strcmpi(azColName[i], colNameRecipientList) == 0)
        {
            recipient = argv[i];
        }
        else if (_strcmpi(azColName[i], colNameRecipientListLen) == 0)
        {
            recipientLen = atoi(argv[i]);
        }
    }


    // OK, now let's find that NC and restore the list

    // code goes in here

    return 0;
}



void RestoreRecipientLists(void)
{
    int rc;
    char *zErrMsg = 0;
    char command[1000];

    if (!PersistRecipientOpenRead())
    {
        log_printf("No persistence database found, nothing will be restored.");
        return;
    }

    // Execute the table query

    sprintf(command, "select * from %s", recipientTableName);
    rc = sqlite3_exec(db, command, RecipientListCallback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        log_lev_printf(LLEV::Release, "SQLite error [%s] Table: \"%s\"", recipientTableName, zErrMsg);
        sqlite3_close(db);
        return;
    }

    PersistRecipientClose();
}

#endif
