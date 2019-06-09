/**************************************************************************
*
*  Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           to original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*       For more information: info@bac-test.com
*       For access to source code:  info@bac-test.com
*               or      www.github.com/bacnettesting/bacnet-stack
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided 
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
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
*
*********************************************************************/

//#ifdef _MSC_VER
//#include <process.h>
//#endif

#include <stdio.h>
#include <string.h>
#include "mqttIPC.h"

/*
 * Prepping the mosquitto library
 * 		git checkout https://github.com/eclipse/mosquitto.git temp
 * 		cd temp
 * 		vi config.mk
 * 			comment out (#) WITH_UUID, WITH_SRV, WITH_DOCS
 * 		make
 * 		sudo make install
 */

#include <mosquitto.h>
#include "mqttIPC.h"
#include "logging.h"
#include "cJSON.h"
#include "bitsUtil.h"
#include "device.h"

extern PORT_SUPPORT *applicationDatalink;
extern PORT_SUPPORT *virtualDatalink;
extern DEVICE_OBJECT_DATA applicationDevice;


// WARNING result is ephemeral
static bool mqGetString(cJSON *operation, const char *parameter, char **result )
{
    cJSON *devInst = cJSON_GetObjectItemCaseSensitive(operation, parameter );
    if (!cJSON_IsString ( devInst ) )
    {
        panic();
        *result = NULL ;
        return false;
    }
    *result = devInst->valuestring;
    return true;
}

static bool mqGetUInt32(cJSON *operation, const char *parameter, uint32_t *result)
{
    cJSON *devInst = cJSON_GetObjectItemCaseSensitive(operation, parameter);
    if (!cJSON_IsNumber(devInst ) )
    {
        panic();
        *result = 0 ;
        return false;
    }
    *result = (uint32_t) devInst->valueint ;
    return true;
}

static bool mqGetFloat(cJSON *operation, const char *parameter, float *result)
{
    cJSON *devInst = cJSON_GetObjectItemCaseSensitive(operation, parameter);
    if (!cJSON_IsNumber(devInst))
    {
        panic();
        *result = 0.;
        return false;
    }
    *result = (float) devInst->valuedouble ;
    return true;
}

static void mqUpdateData(cJSON *operation)
{
    char *objType = NULL;
    uint32_t devInst, objInst ;
    char *dataType = NULL;
    float presentValue;

    if (!mqGetUInt32(operation, "Device Instance", &devInst))
    {
        panic();
        return;
    }

    if (!mqGetString(operation, "Object Type", &objType))
    {
        panic();
        return;
    }

    if (!mqGetUInt32(operation, "Object Instance", &objInst))
    {
        panic();
        return;
    }

    if (!mqGetString(operation, "Data Type", &dataType))
    {
        panic();
        return;
    }

    if (!mqGetFloat(operation, "Present Value", &presentValue))
    {
        panic();
        return;
    }

    printf("Update Data, Dev:%d, Type:%s, Inst:%d, DT:%s, PV:%f\n", devInst, objType, objInst, dataType, presentValue);
}

static void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    // struct mosq_config *cfg;
    // int i;
    // bool res;

    // cfg = (struct mosq_config *)obj;


    printf ( "%s %s\n", message->topic, (char *) message->payload );

    cJSON * root2 = cJSON_Parse( (const char *) message->payload);

    cJSON *operation = cJSON_GetObjectItemCaseSensitive(root2, "Operation");
    if (cJSON_IsString(operation))
    {
        if (isMatchCaseInsensitive( operation->valuestring, "Create Virtual Router") )
        {
        	// get the local port parameters
        	cJSON *paramNN = cJSON_GetObjectItem( root2, "Local Network Number");
        	cJSON *paramBP = cJSON_GetObjectItem( root2, "BACnet Port");
        	if ( cJSON_IsNumber( paramNN ) && cJSON_IsNumber(paramBP))
        	{
                panic(); // restore
#if 0
                bool ok = InitRouterport(PF_BBMD, paramNN->valueint, paramBP->valueint );
                if ( ! ok )
                {
            	    panic();
            	    return ;
                }
#endif
            }
        	else
        	{
        		panic();
        		return ;
        	}

            Device_Init_New(PF_APP, applicationDatalink, &applicationDevice, 12412, "GLP Router Device", "BACnet Virtual Router", NULL);
        }
        if (isMatchCaseInsensitive(operation->valuestring, "Create Virtual Device"))
        {
            DEVICE_OBJECT_DATA *pdev = new DEVICE_OBJECT_DATA();
            Device_Init_New(PF_VIRT, virtualDatalink, pdev, 112412, "GLPvirtualDevice1", "Virtual Device Description", NULL);
        }
        else if (isMatchCaseInsensitive(operation->valuestring, "Update Data"))
        {
            mqUpdateData(root2);
        }
        else
        {
            log_lev_printf(LLEV::Release, "Implement Operation:[%s]", operation->valuestring);
        }
    }

    cJSON_Delete(root2);
}

//#ifdef _MSC_VER
//void mqttThread(void *pArgs)
//#else
//void* mqttThread(void *pArgs)
//#endif
//{
//
//    // start receiving them in a loop.
//    // eventually this loop will have to be in a thread, but for now a while(true) will be OK.
//    int rc = mosquitto_loop_forever((struct mosquitto *) pArgs, -1, 1);
//
//
//    // but we never return,,,, do we?
//    mosquitto_destroy((struct mosquitto *) pArgs);
//    mosquitto_lib_cleanup();
//
//}

int mqttIPCinit ( void )
{
    mosquitto_lib_init();

    int major, minor, rev ;
    mosquitto_lib_version(&major, &minor, &rev);
    printf ("Mosquitto v%d.%d.%d\n", major, minor, rev);

    bool clean_session = true;
    const char *host = "test.mosquitto.org";
    int port = 1883;
    int keepalive = 60;

    struct mosquitto *mosq = mosquitto_new(NULL, clean_session, NULL);
    if ( mosq == NULL )
    {
        printf("Could not init Mosquitto \n");
        mosquitto_lib_cleanup();
        return -1 ;
    }

    if(mosquitto_connect(mosq, host, port, keepalive)){
            fprintf(stderr, "Unable to connect.\n");
            return 1;
        }

#if 0 // publish sanity test
    /*
     * * Parameters:
     * 	mosq -       a valid mosquitto instance.
     * 	mid -        pointer to an int. If not NULL, the function will set this
     *               to the message id of this particular message. This can be then
     *               used with the publish callback to determine when the message
     *               has been sent.
     *               Note that although the MQTT protocol doesn't use message ids
     *               for messages with QoS=0, libmosquitto assigns them message ids
     *               so they can be tracked with this parameter.
     *  topic -      null terminated string of the topic to publish to.
     * 	payloadlen - the size of the payload (bytes). Valid values are between 0 and
     *               268,435,455.
     * 	payload -    pointer to the data to send. If payloadlen > 0 this must be a
     *               valid memory location.
     * 	qos -        integer value 0, 1 or 2 indicating the Quality of Service to be
     *               used for the message.
     * 	retain -     set to true to make the message retained.
     */

    const char *hello = "This is a message from BCE, use echc/subscribe.sh to listen" ;
    mosquitto_publish( mosq, NULL, "MQTT Examples", strlen(hello)+1, hello, 0, 0 ) ;


    // set up a listener on topic "/bcp/0/fb/config" to receive configuration

    // now according to our docs, send something to request a dump of the configuration for devices.
    // this will need a peer mqtt client to receive this and start publishing a sequence of packets

    // to test receipt: mosquitto_sub -h test.mosquitto.org -t /bcp/0/rq/startConfig
    hello = "StartConfig" ;
    mosquitto_publish( mosq, NULL, "bsp/0/bsx/rq", strlen(hello)+1, hello, 0, 0 ) ;
#endif

    // subscribe

    mosquitto_subscribe(mosq, NULL, "/bsp/0/#", 0);

    mosquitto_message_callback_set(mosq, my_message_callback);

//#ifdef _MSC_VER
//    uintptr_t rcode;
//    rcode = _beginthread(mqttThread, 0, (void *)mosq);
//    if (rcode == -1L)
//    {
//        log_lev_printf(LLEV::Debug, "Failed to create thread");
//    }
//#else
//    int rcode;
//    pthread_t threadvar;
//    rcode = pthread_create(&threadvar, NULL,
//        (void *(*)(void *)) router_thread, (void *)port);
//    if (rcode != 0) {
//        log_printf("Failed to create thread");
//    }
//    // so we don't have to wait for the thread to complete before exiting main()
//    pthread_detach(threadvar);
//#endif

    mosquitto_loop_start(mosq);

    return true ;
}

