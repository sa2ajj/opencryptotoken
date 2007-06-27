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

#define FLASHDATA ((__data_load_end+SPM_PAGESIZE-1)/SPM_PAGESIZE)*SPM_PAGESIZE;

static void (*usbPollPtr)(void);

static uchar input[N];
static uchar result[3*N];
 
static unsigned int input_length;
static uchar *write_ptr;

static uchar do_it,result_length,cmd;
unsigned char entrophy[32];

void	usbPoll_() {
  usbPollPtr();
}

uchar   usbAppFunctionSetup(uchar data[8],uchar **usbMsgPtr)
{
    *usbMsgPtr = result;
    cmd=data[1];
    if(cmd == 0){       /* ECHO */
        result[0] = data[2];
        result[1] = data[3];
        return 2;
    } else if(cmd == AVRCMD_MULT || cmd==AVRCMD_SIGN) {
      result_length=0;
      input_length=((usbRequest_t *)data)->wLength.word;
      write_ptr=input;
      memset(input,0,N);
      return 0xff;
    } else if(cmd == AVRCMD_GETRESULT) {
      return result_length;
    } else if(cmd == 6) {
      *usbMsgPtr=entrophy;
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
    delay_ms(20);
    PORTC = PORTC | (1<<0)|(1<<3);
//    PORTC = PORTC | (1<<0);
}


uchar usbAppFunctionWrite(uchar *data, uchar len) {
  int i;
  for(i=0; i<len; i++) *write_ptr++=*data++;
  if(write_ptr-input==input_length) {
    do_it=1;
  }
  return 1;
}

void usbAppInit(void (*ptr)()) {
  usbPollPtr=ptr;
//mrk
  DDRC= 0xff;
  PORTC = 0xff;
  TCCR0 = 1;
  
  ec_init();
  beep();
//  beep();
}

void do_rnd() {

   uchar b;
   static uchar entcnt=0,mask=1;

   DDRC |= _BV(PC5);
   PORTC &= ~_BV(PC5);
//   TCNT0=0x0;

// discharge capacitor 
// (1ms seems be sufficient, how to compute time needed?)
    _delay_ms(1);

//PC5 input dir
    DDRC &= ~_BV(PC5);
//PC5 pull-up resistor, start charging
    PORTC |= _BV(PC5);

    while(!(PINC & _BV(PC5)));
    
//    mask=1<<(entcnt&7);
//    b=entrophy[entcnt/8]&~mask;
//    if(TCNT0&1) b|=mask;
//    entrophy[entcnt/8]=b;
//    entrophy[entcnt/8]^=(TCNT0&1)<<(entcnt&7);
//    entcnt++;
      b=entrophy[entcnt];
//      b&=~mask;
      if(TCNT0&1) b^=mask;
      entrophy[entcnt]=b;

      mask<<=1;
      if(!mask) {
        mask=1;
        entcnt=(entcnt+1)&31;
      }
}
extern unsigned char *P_G;
void usbAppIdle() {
#if 1
  if(do_it) {
    if(cmd==AVRCMD_MULT) {
//      memset(&private_key,0,N);
//      private_key.value[N-1]=0xff;
      ec_mul((ec_point_t *)result,&G,&private_key);
//      memcpy_P(result,&P_G,sizeof(ec_point_t));
      ec_normalize((ec_point_t *)result);
      beep();
      result_length=sizeof(ec_point_t);
    } else if(cmd==AVRCMD_SIGN) {
      int ok=ecdsa_sign((ecdsa_sig_t *)result,input,input_length);
      result_length = ok ? sizeof(ecdsa_sig_t) : 0;
      if(ok) beep();
    }          
    do_it=0;
  }
#endif
  do_rnd();
};

void main() {
}
