/* Name: main.c
 * Project: PowerSwitch based on AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2005-01-16
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: Proprietary, free under certain conditions. See Documentation.
 * This Revision: $Id: main.c 183 2006-03-26 19:07:22Z cs $
 */

#define F_CPU 12000000UL  // 12 MHz
#include <avr/io.h>
#include <string.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/delay.h>

#include "usbdrv.h"
#include "oddebug.h"
#include "ec.h"

/*
This module implements an 8 bit parallel output controlled via USB. It is
intended to switch the power supply to computers and/or other electronic
devices.

Application examples:
- Rebooting computers located at the provider's site
- Remotely switch on/off of rarely used computers
- Rebooting other electronic equipment which is left unattended
- Control room heating from remote
*/

#ifndef TEST_DRIVER_SIZE    /* define this to check out size of pure driver */

static unsigned char entcnt;
unsigned char entrophy[32];
static char input[N];
static unsigned char result[3*N];
 
static unsigned int input_length;
static char *write_ptr;

static uchar do_it,result_length,cmd;

uchar   usbFunctionSetup(uchar data[8])
{
static uchar    replyBuf[2];

    usbMsgPtr = replyBuf;
    cmd=data[1];
    if(cmd == 0){       /* ECHO */
        replyBuf[0] = data[2];
        replyBuf[1] = data[3];
        return 2;
    } else if(cmd == 1){       /* GET_STATUS -> result = 2 bytes */
//        replyBuf[0] = status;
//        replyBuf[1] = computeTemporaryChanges();
        return 0;
    } else if(cmd == AVRCMD_MULT || cmd==AVRCMD_SIGN) {
      result_length=0;
      input_length=((usbRequest_t *)data)->wLength.word;
      write_ptr=input;
      memset(input,0,N);
      return 0xff;
    } else if(cmd == AVRCMD_GETRESULT) {
      usbMsgPtr=result;
      return result_length;
    } else if(cmd == 6) {
      usbMsgPtr=entrophy;
      return 32;
    }
    return 0;
}

/* allow some inter-device compatibility */
#if !defined TCCR0 && defined TCCR0B
#define TCCR0   TCCR0B
#endif
#if !defined TIFR && defined TIFR0
#define TIFR    TIFR0
#endif

/* function for long delay */
void delay_ms(uint16_t ms) {
        while ( ms ) {
              _delay_ms(1);
               ms--;
        }
}

void beep() {
    PORTC = PORTC & ~((1<<0)|(1<<3));
//    PORTC = PORTC & ~((1<<0));
    delay_ms(150);
    PORTC = PORTC | (1<<0)|(1<<3);
//    PORTC = PORTC | (1<<0);
    delay_ms(100);
}


uchar    usbFunctionWrite(uchar *data, uchar len) {
  int i;
  for(i=0; i<len; i++) *write_ptr++=*data++;
  if(write_ptr-input==input_length) {
    do_it=1;
  }
  return 1;
}


int main(void)
{
uchar   i, j;

    wdt_enable(WDTO_1S);
    odDebugInit();
    DDRD = ~(1 << 2);   /* all outputs except PD2 = INT0 */
    PORTD = 0;
//mrk
    DDRC= 0xff;
    PORTC = 0xff;

// Timer 0, CLK without prescaller
    TCCR0 = 1;

    beep();
//    if(ec_test()) beep();    
//    beep();    
    PORTB = 0;          /* no pullups on USB pins */
    DDRB = ~0;          /* output SE0 for USB reset */
    j = 0;
    while(--j){         /* USB Reset by device only required on Watchdog Reset */
        i = 0;
        while(--i);     /* delay >10ms for USB reset */
    }
    DDRB = ~USBMASK;    /* all outputs except USB data */

//    TCCR0 = 5;          /* set prescaler to 1/1024 */
    usbInit();
    sei();

    ec_init();

    for(;;){    /* main event loop */
        usbPoll();
        if(do_it) {
          if(cmd==AVRCMD_MULT) {
            ec_mul(&G,&private_key,(ec_point_t *)result);
            ec_normalize((ec_point_t *)result);
            result_length=sizeof(ec_point_t);
          } else if(cmd==AVRCMD_SIGN) {
            int ok=ecdsa_sign(input,input_length,(ecdsa_sig_t *)result);
//            int ok=ecdsa_sign("dupa",4,(ecdsa_sig_t *)result);
            result_length = ok ? sizeof(ecdsa_sig_t) : 0;
            if(ok) beep();
          }          
          do_it=0;
        }
        {
        unsigned char rnd;
      DDRC |= _BV(PC5);
      PORTC &= ~_BV(PC5);
      TCNT0=0x0;
      _delay_ms(2);

//PC5 input dir
      DDRC &= ~_BV(PC5);
//PC5 pull-up resistor
      PORTC |= _BV(PC5);

      while(!(PINC & _BV(PC5)));
      rnd=TCNT0;
      entrophy[entcnt>>3]^=((rnd&1)<<(entcnt&7));
      entcnt++;
      }
      wdt_reset();
    }
    return 0;
}

#else   /* TEST_DRIVER_SIZE */

/* This is the minimum do-nothing function to determine driver size. The
 * resulting binary will consist of the C startup code, space for interrupt
 * vectors and our minimal initialization. The C startup code and vectors
 * (together ca. 70 bytes of flash) can not directly be considered a part
 * of this driver. The driver is therefore ca. 70 bytes smaller than the
 * resulting binary due to this overhead. The driver also contains strings
 * of arbitrary length. You can save another ca. 50 bytes if you don't
 * include a textual product and vendor description.
 */
uchar   usbFunctionSetup(uchar data[8])
{
    return 0;
}

int main(void)
{
#ifdef PORTD
    PORTD = 0;
    DDRD = ~(1 << 2);   /* all outputs except PD2 = INT0 */
#endif
    PORTB = 0;          /* no pullups on USB pins */
    DDRB = ~USBMASK;    /* all outputs except USB data */
    usbInit();
    sei();
    for(;;){    /* main event loop */
        usbPoll();
    }
    return 0;
}

#endif  /* TEST_DRIVER_SIZE */
