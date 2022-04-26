
#include <stdint.h>
#include "osLayer.h"

// in MSVC, winsock2.h must be included first
#if defined(_MSC_VER)
#include <winsock2.h>
#endif

#if defined(__GNUC__)

#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>

#include <linux/if_link.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#include <stdbool.h>
#include <stdio.h>
#endif

#include "btaDebug.h"
#include "stdint.h"
#include "bacnet/bacstr.h"
#include "eLib/util/emm.h"
//#include "eLib/util/ese.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacaddr.h"

#include "bacnet/bacint.h"
#include "osNet.h"
#include "eLib/util/eLibDebug.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/bits/util/bitsUtil.h"
#include "eLib/util/btaInternal.h"
#include "eLib/util/eLibUtil.h"

// this has been migrated to bitsUtil.h
// #define MIN(x, y) (((x) < (y)) ? (x) : (y))

extern struct sockaddr_in btaLocalIPEP;

// one day set up an observer arrangement here.

void SendBTApayload(uint8_t *payload, const int sendlength)
{
    BTAQ_CB *btaqCB = (BTAQ_CB *)emm_dmalloc('q', sizeof(BTAQ_CB));
    if (btaqCB == NULL) {
        emm_free(payload);
        return;
    }
    btaqCB->payload = payload;
    btaqCB->sendlength = sendlength ;

    SendBTApayloadUDP( btaqCB );
}


#if ( ETHERNETDEBUG == 1 )
// encodes ipendpoint into out buffer
int encodeIPEP(unsigned char *buf, struct sockaddr_in *addr)
{
    memcpy(buf, &addr->sin_addr.s_addr, 4);
    memcpy(&buf[4], &addr->sin_port, 2);
    return 6;
}
#endif


int encodeUInt16(uint8_t *buf, const uint16_t value)
{
    // I have no idea what endianess ST micro is... check it out
#if BACNET_STACK_BIG_ENDIAN
    buf[0] = ((uint8_t *)&value)[0];
    buf[1] = ((uint8_t *)&value)[1];
#else
    buf[0] = ((uint8_t *)&value)[1];
    buf[1] = ((uint8_t *)&value)[0];
#endif
    return 2;
}


int encodeUInt32(unsigned char *buf, const uint32_t val)
{
#if BACNET_STACK_BIG_ENDIAN
    buf[0] = ((uint8_t *)&val)[0];
    buf[1] = ((uint8_t *)&val)[1];
    buf[2] = ((uint8_t *)&val)[2];
    buf[3] = ((uint8_t *)&val)[3];
#else
    buf[0] = ((uint8_t *)&val)[3];
    buf[1] = ((uint8_t *)&val)[2];
    buf[2] = ((uint8_t *)&val)[1];
    buf[3] = ((uint8_t *)&val)[0];
#endif
    return 4;
}

#if defined(_MSC_VER)

// Clock gettime for win32 from StackOverflow: http://goo.gl/WHR94q

LARGE_INTEGER getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

void clock_gettime(struct timeval *tv)
{
    LARGE_INTEGER           t;
    FILETIME				f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    // static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        //LARGE_INTEGER performanceFrequency;
        //initialized = 1;
        //usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        //if (usePerformanceCounter) {
        //	QueryPerformanceCounter(&offset);
        //	frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        //} else {
        offset = getFILETIMEoffset();
        frequencyToMicroseconds = 10.;
        //}
    }

    //if (usePerformanceCounter) QueryPerformanceCounter(&t);
    //else {
    GetSystemTimeAsFileTime(&f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    //}

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = (LONGLONG)microseconds;

    // t.QuadPart -= 116444736000000000 ;

    tv->tv_sec = (LONG)(t.QuadPart / 1000000);
    tv->tv_usec = t.QuadPart % 1000000;
}
#endif

// BTA reporting packet frame structure can be found here: https://goo.gl/JixmbT

void PrepareBTAheader(const BTApt fc, uint8_t *buffer)
{
    unsigned long tosendlong;

    buffer[0] = 0;
    buffer[1] = fc;

    // uint32_t    ourAddr;

#if _MSC_VER
    struct timeval gettime_now;

    // this is a very high resolution time, but from start of execution, not UTC... todo 4 - combine..
    clock_gettime(&gettime_now);

    // time
    tosendlong = htonl(gettime_now.tv_sec);
    memcpy(&buffer[2], &tosendlong, 4);

    tosendlong = htonl(gettime_now.tv_usec * 1000);
    memcpy(&buffer[6], &tosendlong, 4);
#endif

#ifdef SOMELINUX
    // this is better, returning to prior method for a while until this subject
    // is revisited (so as to highlight that it is incomplete).

    SYSTEMTIME st;
    FILETIME ft;
    ULONGLONG hectomicrosecs;       // 64-bit

    long seconds;
    long milliseconds;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

    hectomicrosecs = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    // counts of 100ns intervals since Jan 1, 1601, and reduced to seconds,
    // seconds won't fit in 32 bit, so do offset to linux time.. 1970-01-01
    // 00:00:00 +0000 (UTC) is 1970 - 1601 = 369 years = 369 * 31,556,925.9936
    // seconds = 11644505691.6384 seconds

    hectomicrosecs -= 116445056916384 * 1000;

    // time
    // seconds - remember - UTC !
    tosendlong = (unsigned long)(hectomicrosecs / 10000000);
    tosendlong = htonl(tosendlong);
    memcpy(&buffer[2], &tosendlong, 4);

    // time
    // nanosecs
    tosendlong = (long)((hectomicrosecs * 100) % 1000000000);
    tosendlong = htonl(tosendlong);
    memcpy(&buffer[6], &tosendlong, 4);
#endif

#if ( ETHERNETDEBUG == 1 )
#if ! defined ( _MSC_VER  ) 	// some linux
    struct timespec gettime_now;

    clock_gettime(CLOCK_REALTIME, &gettime_now);

    // time
    tosendlong = htonl(gettime_now.tv_sec);
    memcpy(&buffer[2], &tosendlong, 4);
    tosendlong = htonl(gettime_now.tv_nsec);
    memcpy(&buffer[6], &tosendlong, 4);
#endif
#endif

    // remote agent IP
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
//    ourAddr = bip_get_addr();
    // ourAddr = 0;

    // this trick only works if we know the destination of the peer....
    // i.e. we should wait for a ping from BTA before trying to establish local
    // address/interface..
    // struct sockaddr_in sin;
    // TryGetLocalAddress(&sin);
    // ourAddr = sin.sin_addr.s_addr;

    // todo3 - see
    // http://stackoverflow.com/questions/2432493/how-to-detemine-which-network-interface-ip-address-will-be-used-to-send-a-pack
    // encodeUInt32(&buffer[10], ourAddr);
    // don't bother with the port, it is not used by BTA anyway.

    encodeUInt32(&buffer[10], ntohl(btaLocalIPEP.sin_addr.s_addr));
    encodeUInt16(&buffer[14], ntohs(btaLocalIPEP.sin_port));

#else
    encodeIPEP(&buffer[10], &ourAddress);
#endif

}


void SendBTAhexdump(const char *message, const void *buffer, const uint16_t len)
{
    // if ( ! BTA_Ready() ) return ;
    uint8_t *outbuf = (uint8_t *)emm_smalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return;
    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(BTApt_Hexdump, outbuf);

    BACNET_CHARACTER_STRING tstring;
    characterstring_init_ansi(&tstring, message);
    uint16_t apdu_len = encode_application_character_string(&outbuf[20], &tstring);

    encodeUInt16(&outbuf[16], apdu_len);           // space allocated for message

    // if there is not enough space, trim the len
    uint16_t newlen = BITS_MIN(len, MX_BTA_BUFFER - 20 - apdu_len);
    encodeUInt16(&outbuf[18], newlen);

    memcpy(&outbuf[20 + apdu_len], buffer, newlen);
    SendBTApayload(outbuf, 20 + apdu_len + newlen);
}


static void SendBTAtext(const BTApt pt, const char *message)
{
    static bool recurseFlag;
    if (recurseFlag) return;
    recurseFlag = true;

    uint8_t *outbuf = (uint8_t *)emm_smalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return;
    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(pt, outbuf);
    int datalen = BITS_MIN(bits_strlen(message), MX_BTA_BUFFER - BX_DATAOFFSET - 1);
    strncpy((char *)&outbuf[BX_DATAOFFSET], message, datalen);
    SendBTApayload(outbuf, datalen + BX_DATAOFFSET);

    recurseFlag = false;
}

void SendBTAmessage(const char *message)
{
    SendBTAtext(BTApt_TextMsg, message);
}

void SendBTAstartMessage(const char *message)
{
    SendBTAtext(BTApt_StartMsg, message);
}

void SendBTApanicMessage(const char *message)
{
    SendBTAtext(BTApt_PanicMsg, message);
}

#define MX_TMESSAGE     100

static void EncodeInt(char *tmessage, int *optr, int value)
{
    tmessage[(*optr)++] = ((value / 1000) % 10) + '0';
    tmessage[(*optr)++] = ((value / 100) % 10) + '0';
    tmessage[(*optr)++] = ((value / 10) % 10) + '0';
    tmessage[(*optr)++] = ((value / 1) % 10) + '0';
    tmessage[(*optr)++] = 0;
}

void SendBTApanicInt(const char *message, const int value)
{
  // if ( ! BTA_Ready() ) return ;

    char tmessage[MX_TMESSAGE];
    int len = bits_strlen(message);
    if (len > MX_TMESSAGE - 10) return;
    strncpy(tmessage, message, MX_TMESSAGE);
    tmessage[len++] = ' ';
    tmessage[len++] = ' ';
    EncodeInt(tmessage, &len, value);
    SendBTApanicMessage(tmessage);
}

void SendBTAmessageF1(char *message, int value)
{
    char tmessage[MX_TMESSAGE];
  // if ( ! BTA_Ready() ) return ;
    int len = bits_strlen(message);
    if (len > MX_TMESSAGE - 10) return;
    strncpy(tmessage, message, MX_TMESSAGE);
    tmessage[len++] = ' ';
    tmessage[len++] = ' ';
    EncodeInt(tmessage, &len, value);
    tmessage[len++] = 0;
    SendBTAmessage(tmessage);
}


void SendBTAmstpFrame( const uint8_t portId, const bool tx, const uint8_t *frame, const uint16_t odata_len )
{
    // do this check early to avoid unnecessary CPU
  // if ( ! BTA_Ready() ) return ;

    if (odata_len > 512 || odata_len < 2) {
        // a bit recursive? panic();
        return;
    }

    // we are not going to send the 55, ff,
    // we are not going to send the 55, ff,
    uint tdata_len = odata_len - 2 ;
    // unsigned char outbuf[MX_BTA_BUFFER];
    uint8_t *outbuf = (uint8_t *)emm_smalloc('c', MX_BTA_BUFFER);
    if (outbuf == NULL) return ;

    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(BTApt_MSTPframe, outbuf);
    
    outbuf[16] = tx ;
    outbuf[17] = portId;
    
    int datalen = BITS_MIN(tdata_len, MX_BTA_BUFFER - BX_DATAOFFSET);
    memcpy((uint8_t *)&outbuf[BX_DATAOFFSET], &frame[2], datalen);
    SendBTApayload(outbuf, datalen + BX_DATAOFFSET);
}

// and if we don't have the header information
void SendBTAmstpPayload(
    const uint8_t portId,
    const bool tx,
    const uint16_t data_len,
    const uint8_t function,
    const uint8_t dest,
    const uint8_t source,
    const uint8_t *payload)
{
//        if ( ! BTA_Ready() ) return;

    uint8_t *outbuf = (uint8_t *)emm_smalloc('p', MX_BTA_BUFFER);
    if (outbuf == NULL) return;

    memset(outbuf, 0, MX_BTA_BUFFER);
        
    // BTA reporting packet frame structure can be found here: https://goo.gl/JixmbT
        
    PrepareBTAheader(BTApt_MSTPframe, outbuf);
        
    outbuf[16] = tx ;
    outbuf[17] = portId;
        
    int datalen = BITS_MIN(data_len, MX_BTA_BUFFER - BX_DATAOFFSET - 6);

    // this sends a whole frame
    outbuf[BX_DATAOFFSET + 0] = function;
    outbuf[BX_DATAOFFSET + 1] = dest;
    outbuf[BX_DATAOFFSET + 2] = source;
    outbuf[BX_DATAOFFSET + 3] = (uint8_t)(data_len >> 8);
    outbuf[BX_DATAOFFSET + 4] = (uint8_t)(data_len & 0xff);
    outbuf[BX_DATAOFFSET + 5] = 0;    // header checksum
    memcpy(&outbuf[BX_DATAOFFSET + 6], payload, datalen);

    SendBTApayload(outbuf, datalen + BX_DATAOFFSET + 6);
}

static void SendBTApacketTxRx(const int port_id,
    const BACNET_MAC_ADDRESS *srcPhyMac,
    const BACNET_MAC_ADDRESS *destPhyMac,
    const uint8_t *pdu,
    const int len,
    const int flagtx)
{
    //     unsigned char outbuf[1500];
    int i;
    BACNET_MAC_ADDRESS localSrcPhyMac, localDestPhyMac;

    uint8_t *outbuf = (uint8_t *)emm_smalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return;

    bacnet_mac_copy(&localSrcPhyMac, srcPhyMac);
    bacnet_mac_copy(&localDestPhyMac, destPhyMac);

#if ( BAC_DEBUG == 1 )
    if (!bacnet_mac_check(&localSrcPhyMac)) {
        panic();
        bacnet_mac_clear(&localSrcPhyMac);
    }
    if (!bacnet_mac_check(&localDestPhyMac)) {
        panic();
        bacnet_mac_clear(&localDestPhyMac);
    }
#endif

    PrepareBTAheader(BTApt_BACstd, outbuf);

    int optr = 16;
    // prep the payload

    // flags... Rx
    outbuf[optr++] = (flagtx) ? 1 : 0;

    // process ID

    optr += encodeUInt16(&outbuf[optr], (uint16_t)port_id);

    // source
    outbuf[optr++] = localSrcPhyMac.len;
    for (i = 0; i < localSrcPhyMac.len; i++) {
        outbuf[optr++] = localSrcPhyMac.bytes[i];
    }

    // dest
    outbuf[optr++] = localDestPhyMac.len;
    for (i = 0; i < localDestPhyMac.len; i++) {
        outbuf[optr++] = localDestPhyMac.bytes[i];
    }

    // now the packet itself
    memcpy(&outbuf[optr], pdu, len);
    optr += len;

    SendBTApayload(outbuf, optr);
}


void SendBTApacketTx(
    const int port_id,
    const BACNET_MAC_ADDRESS *srcPhyMac,
    const BACNET_MAC_ADDRESS *destPhyMac,
    const uint8_t *pdu,
    const int len)
{
    SendBTApacketTxRx(port_id, srcPhyMac, destPhyMac, pdu, len, 1);
}


void SendBTApacketRx(
    const int port_id,
    const BACNET_MAC_ADDRESS *srcPhyMac,
    const BACNET_MAC_ADDRESS *destPhyMac,
    const uint8_t *pdu,
    const int len)
{
    SendBTApacketTxRx(port_id, srcPhyMac, destPhyMac, pdu, len, 0);
}


