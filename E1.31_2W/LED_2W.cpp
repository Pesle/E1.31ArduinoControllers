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
|| | An Arduino Library (ATMEGA328p) that outputs signals to a L293D/L298D
|| | H-Bridge for popular 2 wire Christmas lights
|| |
|| | It uses a single hardware timer (Timer 2) to control up to 4 channels
|| |
|| #
||
|| @name LED 2W Library
|| @type Library
|| @target Atmel AVR 8 Bit
||
|| @version 2.1.0
||
*/

#include "LED_2W.h"

//CONSTRUCTOR
LED_2W::LED_2W(int en, int in1, int in2){
  brightness1 = 0;
  brightness2 = 0;
  
  enable = en;
  input1 = in1;
  input2 = in2;
  
  pinMode(enable, OUTPUT);
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
}

bool LED_2W::fadeOnAll(int bright, int steps){
  if(fading == false){  //Check if its not currently fading
    fading = true;      //Set fading
    brightness1 = 0;    
    brightness2 = 0;
  }else{
    //Increase brightness until it reaches 'bright'
    if(brightness1 < bright){
      brightness1 += steps;
      brightness2 += steps;
    }else{
      fading = false;   //Set fading
    }
  }
  return fading;  //Return State of Fading
}

bool LED_2W::fadeTo(int bright, int steps, bool second){
  if(fading == false){  //Check if its not currently fading
    fading = true;      //Set fading
    brightness1 = 0;
    brightness2 = 0;
  }else{
    //Increase brightness until it reaches 'bright' for either channel
    if(second){
      if(brightness2 < bright){
        brightness2 += steps;
      }else{
        fading = false;
      }
    }else{
      if(brightness1 < bright){
        brightness1 += steps;
      }else{
        fading = false;
      }
    }
  }
  return fading;  //Return State of Fading
}

bool LED_2W::fadeOffAll(int steps){
  if(brightness1 > 0 || brightness2 > 0){ //Check if a channel is above 0
    //Decrease Brightness
    if(brightness2 > 0){
      brightness2 -= steps;
    }
    if(brightness1 > 0){
      brightness1 -= steps;
    }
    fading = true;
  }else{
    fading = false;
    brightness1 = 0;
    brightness2 = 0;
  }
  return fading;  //Return State of Fading
}

//Set brightness for all
void LED_2W::set(int bright){
  brightness1 = bright;
  brightness2 = bright;
}

//Set brightness for either
void LED_2W::set(int bright, bool second){
  if(!second){
    brightness1 = bright;
  }else{
    brightness2 = bright;
  }
}

//Show the brightness for either
void LED_2W::show(bool second){
  if(!second){
    on(brightness1,false);
  }else{
    on(brightness2,true);
  }
}

//Turn off the LEDs
void LED_2W::off(bool second){
  if(second == false){
    digitalWrite(input1, LOW);
  }else{
    digitalWrite(input2, LOW);
  }
}

//Turn on the LEDs
void LED_2W::on(uint8_t  brightness, bool second){
  if(brightness > 1){
    analogWrite(enable, brightness);    //Set brightness PWM
    digitalWrite(input1, second);       //Set inputs
    digitalWrite(input2, ! second);
  }else{
    off(second);   //Remove slight glow
  }
}
