// ---E1.31 to 2 Wire Lights---
// This code uses my own LED_2W library to use a H-Bridge to control the output
// of newer 2 wire Christmas Lights that need inverting signals

// LED_2W H-Bridge LED Driver code By Ryan Jobse (pesle@email.com)
// This code may be freely distributed and used as you see fit for non-profit
// purposes and as long as the original author is credited and it remains open
// source

// E1.31 Receiver by Andrew Huxtable (andrew@hux.net.au)
// This code may be freely distributed and used as you see fit for non-profit
// purposes and as long as the original author is credited and it remains open
// source

// Data connections to the module:
// Module -> Arduino
// SI -> 11
// SO -> 12
// SCK -> 13
// CS -> 16
//
// NB Most of the modules run on 3.3v, not 5v - Use caution! 5V logic/data is fine though.
//
// Please configure your Lighting product to use Unicast to the IP the device is given from your DHCP server
// Multicast is not currently supported due to bandwidth/processor limitations

// You will need the Ethercard Library from:
// https://github.com/jcw/ethercard
// 

// There is deliberately no serial based reporting from the code to conserve SRAM. There is a **little**
// bit available if you need to add some in for debugging but keep it to an absolute minimum for debugging
// only.

#include "LED_2W.h"
#include "LED_2W_timer.h"

#include <EtherCard.h>
//#include <avr/wdt.h>

//************************************************** *******************************

// enter desired universe and subnet (sACN first universe is 1)
#define DMX_SUBNET 0
#define DMX_UNIVERSE 1 //**Start** universe

// Set a different MAC address for each...
const uint8_t  mymac[] = { 0x5D, 0x86, 0x05, 0x88, 0x33, 0x02 };

//************************************************** *****
// Ethernet interface ip address
static uint8_t  myip[] = { 10,0,10,22};
//************************************************** *****

// By sacrificing some of the Ethernet receive buffer, we can allocate more to the LED array

/// DONT CHANGE unless you know the consequences...
#define ETHERNET_BUFFER 200   //Low ethernet buffer allows more time on showing the lights
#define CHANNEL_COUNT 8
#define OUTPUT_COUNT 4
#define UNIVERSE_COUNT 1
#define LEDS_PER_UNIVERSE 8

bool flipped = false;

//****** SETUP L293Ds HERE ******

//Enable Pin, Input 1 Pin, Input 2 Pin

LED_2W* e[OUTPUT_COUNT] = {
  new LED_2W(10,0,1),
  new LED_2W(5,2,4),
  new LED_2W(6,7,8),
  new LED_2W(9,14,15)
};

//************************************************** ******************************

unsigned long lastUpdate;

// Define the array of leds

uint8_t  Ethernet::buffer[ETHERNET_BUFFER]; // tcp/ip send and receive buffer

 int checkACNHeaders(const char* messagein, int messagelength) {
  lastUpdate = millis();
  //Do some VERY basic checks to see if it's an E1.31 packet.
  //uint8_t s 4 to 12 of an E1.31 Packet contain "ACN-E1.17"
  //Only checking for the A and the 7 in the right places as well as 0x10 as the header.
  //Technically this is outside of spec and could cause problems but its enough checks for us
  //to determine if the packet should be tossed or used.
  //This improves the speed of packet processing as well as reducing the memory overhead.
  //On an Isolated network this should never be a problem....
  //for (int i=0;i<125;i++){
  // Serial.print(messagein[i]);
  //}
  if ( messagein[1] == 0x10 && messagein[4] == 0x41 && messagein[12] == 0x37) { 
    int addresscount = (uint8_t ) messagein[123] * 256 + (uint8_t ) messagein[124]; // number of values plus start code
    if ( addresscount > 0){
      //Serial.println(addresscount - 1);
      return addresscount -1; //Return how many values are in the packet.
    }
  }
  return 0;
}

void sacnDMXReceived(const char* pbuff, int count) {
  //Serial.println("*");
  if (count > CHANNEL_COUNT) count = CHANNEL_COUNT;
    uint8_t  b = pbuff[113]; //DMX Subnet
    if ( b == DMX_SUBNET) {
      b = pbuff[114]; //DMX Universe
      if ( b >= DMX_UNIVERSE && b <= DMX_UNIVERSE + UNIVERSE_COUNT ) { 
        if ( pbuff[125] == 0 ) { //start code must be 0
        int ledNumber = (b - DMX_UNIVERSE) * LEDS_PER_UNIVERSE;

        //Skip ahead by 2s for each channel
        for (int i = 126;i < 126+count;i = i + 2){
          uint8_t  charValue1 = pbuff[i];
          uint8_t  charValue2 = pbuff[i+1];
          e[ledNumber]->set(charValue1, false);
          e[ledNumber]->set(charValue2, true);
          ledNumber++;
        }
      }
    }
  }
}
  
  static void sACNPacket(unsigned int port, uint8_t  ip[4], unsigned int i, const char *data, unsigned int len) {
  //Make sure the packet is an E1.31 packet
  //Serial.println("*");
  int count = checkACNHeaders(data, len);
  if (count){
    // It is so process the data to the LEDS
    //Serial.println(count);
    sacnDMXReceived(data, count);
  }
}

void initTest() //runs at board boot to make sure the lights are working
{
  for(int i=0;i < 255; i++){
    for(int j=0; j < OUTPUT_COUNT; j++){
      e[j]->fadeOnAll(255, 1);
    }
    delay(4);
  }
  delay(1000);
  for(int i=255;i >= 0; i--){
    for(int i=0;i < OUTPUT_COUNT; i++){
      e[i]->fadeOffAll(1);
    }
    delay(4);
  }
}

//Timer Interrupt
ISR(TIMER_INTERRUPT){
  for(int i=0;i < OUTPUT_COUNT; i++){
    e[i]->show(flipped);
  }
  flipped = !flipped;
}

void setup() {

TIMER_INIT(OCR);  //Start the Timer

//Test the lights
//Also works as a 'backup' if the ethernet disconnects
initTest();

//Serial.begin(115200);
//Serial.println("Init.");

// No checks in here to ensure Ethernet initiated properly.
// Make sure Ethernet cable is connected and there is DHCP on the network if you are using it ....
if (ether.begin(sizeof Ethernet::buffer, mymac, 16) == 0){
    //Failed to access ethernet controller
} else {
  //Serial.println("Ethernet ok");
}
// ************************************************** ****** 
// DHCP
//ether.dhcpSetup();

//Static IP
ether.staticSetup(myip);
// ************************************************** ******

// Register listener
ether.udpServerListenOnPort(&sACNPacket, 5568);

lastUpdate = millis();
}

void loop() {
//Process packets
  if (millis() - lastUpdate > 2000){
  // If no packet recieved in 2 seconds, reboot.
  // I had problems with occational lockups of controllers
  // for some reason. Slowing the data rate seemed to help.
  // ENC28J60 module probably isnt coping too well.
    asm volatile (" jmp 0");
  }
  ether.packetLoop(ether.packetReceive());
}
