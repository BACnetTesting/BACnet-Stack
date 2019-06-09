/**************************************************************************
*
* Copyright (C) 2014 ConnectEx, Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* Should this license not be acceptable, then all use of this software
* must be immediately terminated, and all copies deleted.
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
*********************************************************************/

#include "config.h"

#if ( BITS_USE_FLASHERR == 1 )

#define ESE_DEBUG

#ifdef ESE_DEBUG

#include "ese.h"
#include "led.h"

#define MX_SYS_ERR  10

// time intervals below are 10ms units, ie. 30 = 300ms

#define  TmrHeader              200
#define  TmrHeaderOff           150

#define  TmrFlashOn             48  // 48 38 38 28
#define  TmrFlashOff            24  // 24 24 18 18

#define  TmrInterDigit          150 // 160
#define  TmrTail                200

#define  TmrInterValueFlash     2
#define  TmrInterValue          200


static uint8_t sysErr[MX_SYS_ERR] ;
static uint8_t sysErrCount, ledErrTail;
static uint8_t ledTimer, sysErrState, flashes, val, divider, digit;


static void LedOn( void )
    {
        led_on(SPARE_LED_USB_RED ); 
    }

static void LedOff( void )
    {
        led_off(SPARE_LED_USB_RED); 
    }

void FlashErr_clear ( void )
{
    int i ;
    LedOff();
    sysErrCount = 0 ;
    ledErrTail = 0 ;
    sysErrState = 0 ;
    for ( i=0; i<MX_SYS_ERR; i++)
    {
        sysErr[i]= (uint8_t) 0  ;
    }
}


void FlashErr_enqueue( uint8_t value )
{
    if ( sysErrCount == MX_SYS_ERR ) return ;
    sysErr[sysErrCount++] = value ;
}

//void FlashErr_enqueue_once( uint8_t value)
//{
//    uint8_t i = 0 ;
//    while ( i != sysErrCount )
//        {
//        if ( sysErr[i++] == value ) return;
//        }
//    LedOn();    // todo 3 - remove when scope trigger no longer needed
//    FlashErr_enqueue(value);
//}


static void GetNextVal( void )
    {
    if (ledErrTail == sysErrCount) return;

    val = sysErr[ledErrTail++];
    divider = 100 ;
    do
        {
        flashes = (uint8_t) ( val / divider );
        if ( flashes )
            {
            val -= (uint8_t) ( flashes * divider ) ;
            break;
            }
        divider /= 10;
        }
    while (divider > 0 );
    }


// Call this every 1 ms to drive flaserr
void FlashErr1msTick(void)
{
    static int countdown = 0 ;
    if ( countdown > 1 )
    {
        countdown--;
    }
    else
    {
        FlashErr10msTick();
        countdown = 10 ;
    }
}

void FlashErr10msTick(void)
    {
    if (ledTimer > 1)
        {
        ledTimer--;
        return;
        }

    switch (sysErrState)
        {
        case 0: // Output leading 'dash'
            if (!ese_available()) return;
            ledErrTail = 0 ;
            LedOn();
            ledTimer = TmrHeader;
            goto nextState;
        case 1:
            LedOff();
            ledTimer = TmrHeaderOff;
            GetNextVal();
            goto nextState;
        case 2: // Output next value
            if (flashes == 0)
                {
                // all over for this digit, are there more?
                if (val > 0)
                    {
                    // yes, inter-digit space
                    ledTimer = TmrInterDigit;
                    divider /= 10 ;
                    flashes = val / divider;
                    val -= flashes * divider;
                    ledTimer = TmrInterDigit;
                    // but stay in this state
                    }
                else
                    {
                    // any more values?
                    GetNextVal();
                    ledTimer = TmrInterValue;
                    // stay in this state in case there are (val, flashes set to non-zero)
                    if (flashes == 0)
                        {
                        sysErrState = 0;
                        ledTimer = TmrTail;
                        }
                    else
                        {
                        // // stay in this state
                        ledTimer = TmrInterValue;
                        sysErrState = 10;
                        }
                    }
                return;
                }
            LedOn();
            ledTimer = TmrFlashOn;
            goto nextState;
        case 3:
            LedOff();
            ledTimer = TmrFlashOff;
            flashes--;
            sysErrState = 2;
            break;
        case 10: // inter value blip
            LedOn();
            ledTimer = TmrInterValueFlash;
            goto nextState;
        case 11:
            LedOff();
            ledTimer = TmrInterValue;
            sysErrState = 2;
            break;
        }

    return;

nextState:
    sysErrState++;
    }


#endif // ESE_DEBUG

#endif