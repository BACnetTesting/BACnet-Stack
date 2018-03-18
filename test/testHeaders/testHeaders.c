#define BBMD_ENABLED            1
#define MAX_TSM_TRANSACTIONS    10
/*
    This project is here to enable us to test that the header files will compile on their own... 
    in order to validate dependencies 

    http://www.acodersjourney.com/2016/05/top-10-c-header-file-mistakes-and-how-to-fix-them/

*/

/*

    Also, go to menu->project->properties->output->show includes

*/

// #include "listmanip.h"
// #include "device.h"
// #include "bbmd.h"
// #include "bvlc.h"
// #include "msgqueue.h"
// #include "bacdef.h"
// #include "bacenum.h"
// #include "npdu.h"
// #include "linklist.h"
// #include "portThread.h"
// #include "net.h"
// #include "bip.h"
// #include "datalink.h"
// #include "CEDebug.h"
// #include "tsm.h"
// #include "handlers.h"
// #include "alarm_ack.h"
// #include "rp.h"
// #include "wp.h"
// #include "client.h"
// #include "event.h"
// #include "config.h"
// #include "bacstr.h"
// #include "datetime.h"
// #include "readrange.h"
// #include "lso.h"
// #include "timesync.h"
// #include "msgqueue.h"
// #include "dlmstp.h"
// #include "bacaddr.h"

// #include "bacnetObject.h"
// #include "ai.h"
#include <stdint.h>
#include <stdbool.h>
#include "ringbuf.h"

volatile uint8_t *buffer;

int main()
{
    
}
