#include <stdio.h>
#include <string.h>

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

int main(void) {

	printf("hello\n");

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
		printf("Could not init\n");
		return -1 ;
	}

	if(mosquitto_connect(mosq, host, port, keepalive)){
			fprintf(stderr, "Unable to connect.\n");
			return 1;
		}


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
	// todo pww
	// this will need a peer mqtt client to receive this and start publishing a sequence of packets

	// to test receipt: mosquitto_sub -h test.mosquitto.org -t /bcp/0/rq/startConfig
	hello = "Start" ;
	mosquitto_publish( mosq, NULL, "/bcp/0/rq/startConfig", strlen(hello)+1, hello, 0, 0 ) ;

	// todo pww and start receiving them in a loop.
	// eventually this loop will have to be in a thread, but for now a while(true) will be OK.

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

}
