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
|| | You cannot use Pin 3 or 11 for Enable as using Timer 2 deactivates them for PWM
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

#include <Arduino.h>
#include <stdint.h>

class LED_2W{
  public:
    LED_2W(int en, int in1, int in2);     //Enable, input1 and input2 pins

    
    bool fadeOnAll(int bright, int steps);            //Fades all from off to on
    bool fadeTo(int bright, int steps, bool second);  //Fades either from off to 'bright'
    bool fadeOffAll(int steps);                       //Fades all from where it is to off

    void set(int bright);                 //Sets the brightness of BOTH channels
    void set(int bright, bool second);    //Sets the brightness of either channel
    
    void show(bool second);               //Used by the timer

  private:
    void off(bool second);                       //Turns the output off
    void on(uint8_t brightness, bool second);    //Turns the output on
  
    int brightness1;    //Brightness of channel 1
    int brightness2;    //Brightness of channel 2
    bool fading;        //Checks if it is currently fading

    //PINS
    int enable;
    int input1;         
    int input2;
};
