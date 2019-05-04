/*
|| @author         Ryan Jobse
|| @url            http://www.pesle.info
||
|| @description
|| | A 2 Wire Christmas Light Library
|| |
|| | Written by Ryan Jobse
|| | http://www.pesle.info
|| |
|| | A simple Timer setup for Timer 2
||
|| @name LED 2W Library
|| @type Library
|| @target Atmel AVR 8 Bit
||
|| @version 2.1.0
||
*/

//DO NOT TOUCH, MAY BECOME UNSTABLE!

//Set OCR to the highest stable number between 0-255. 
//The ethernet card will not work if the value is too high or too low.
#define OCR 240
#define TIMER_INTERRUPT    TIMER2_COMPA_vect
#define TIMER_INIT(ocr) ({\
  TIFR2 = (1 << TOV2);                                  /* clear interrupt flag */ \
  TCCR2A = 0;                                           /* set entire TCCR2A register to 0 */ \
  TCCR2B = 0;                                           /* same for TCCR2B */ \
  TCNT2  = 0;                                           /* initialize counter value to 0 */ \
  TCCR2B |= (1 << CS22) | (1 << CS21) | (0 << CS20);    /* start timer (ck/256 prescalar) */ \
  TCCR2A = (1 << WGM21);                                /* CTC mode */ \
  OCR2A = (ocr);                                        /* Highest number possible that is stable (higher frequency) */ \
  TIMSK2 = (1 << OCIE2A);                               /* enable timer2 output compare match interrupt */ \
})
