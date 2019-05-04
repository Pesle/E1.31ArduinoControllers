// ---E1.31 to Standard Outputs---
// This code uses SoftPWM to get 12 output channels which can use either
// use MosFETS or transistors to output to Christmas Lights

// E1.31 Receiver and pixel controller by Andrew Huxtable (andrew@hux.net.au)
// This code may be freely distributed and used as you see fit for non-profit
// purposes and as long as the original author is credited and it remains open
// source
//
// The ethernet module is an ENC28J60 based item which are easily available
// for a few $
//
// Data connections to the module:
// Module -> Arduino
// SI -> 11
// SO -> 12
// SCK -> 13
// CS -> 10
//
// NB Most of the modules run on 3.3v, not 5v - Use caution! 5V logic/data is fine though.
//
// Please configure your Lighting product to use Unicast to the IP the device is given from your DHCP server
// Multicast is not currently supported due to bandwidth/processor limitations

// You will need the Ethercard and FastLed Libraries from:
// https://github.com/FastLED/FastLED/releases
// https://github.com/jcw/ethercard
//
// The Atmega328 only has 2k of SRAM. This is a severe limitation to being able to control large
// numbers of smart pixels due to the pixel data needing to be stored in an array as well as
// a reserved buffer for receiving Ethernet packets. This code will allow you to use a maximum of 240 pixels
// as that just about maxes out the SRAM on the Atmega328.

// There is deliberately no serial based reporting from the code to conserve SRAM. There is a **little**
// bit available if you need to add some in for debugging but keep it to an absolute minimum for debugging
// only.

#include <EtherCard.h>
//#include <avr/wdt.h>
  
#include "SoftPWM.h"

//************************************************** *******************************

// enter desired universe and subnet (sACN first universe is 1)
#define DMX_SUBNET 0
#define DMX_UNIVERSE 1 //**Start** universe

// Set a different MAC address for each...
const byte mymac[] = { 0x5D, 0x86, 0x05, 0x88, 0x32, 0x01 };

// Uncomment if you want to use static IP
//************************************************** *****
// ethernet interface ip address
static byte myip[] = { 10,0,10,11};
//************************************************** *****

// By sacrificing some of the Ethernet receive buffer, we can allocate more to the LED array
// but this is **technically** slower because 2 packets must be processed for all 240 pixels.

/// DONT CHANGE unless you know the consequences...
#define ETHERNET_BUFFER 200
#define CHANNEL_COUNT 12
#define UNIVERSE_COUNT 1
#define LEDS_PER_UNIVERSE  12

const int pins[] = {2,3,4,5,6,7,8,9,14,15,16,17};

//************************************************** ******************************

unsigned long lastUpdate;

// Define the array of leds

byte Ethernet::buffer[ETHERNET_BUFFER]; // tcp/ip send and receive buffer

 int checkACNHeaders(const char* messagein, int messagelength) {
  lastUpdate = millis();
  //Do some VERY basic checks to see if it's an E1.31 packet.
  //Bytes 4 to 12 of an E1.31 Packet contain "ACN-E1.17"
  //Only checking for the A and the 7 in the right places as well as 0x10 as the header.
  //Technically this is outside of spec and could cause problems but its enough checks for us
  //to determine if the packet should be tossed or used.
  //This improves the speed of packet processing as well as reducing the memory overhead.
  //On an Isolated network this should never be a problem....
  //for (int i=0;i<125;i++){
  // Serial.print(messagein[i]);
  //}
  if ( messagein[1] == 0x10 && messagein[4] == 0x41 && messagein[12] == 0x37) { 
    int addresscount = (byte) messagein[123] * 256 + (byte) messagein[124]; // number of values plus start code
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
    byte b = pbuff[113]; //DMX Subnet
    if ( b == DMX_SUBNET) {
      b = pbuff[114]; //DMX Universe
      if ( b >= DMX_UNIVERSE && b <= DMX_UNIVERSE + UNIVERSE_COUNT ) { 
        if ( pbuff[125] == 0 ) { //start code must be 0
        int ledNumber = (b - DMX_UNIVERSE) * LEDS_PER_UNIVERSE;
        // sACN packets come in seperate RGB but we have to set each led's RGB value together
        // this 'reads ahead' for all 3 colo urs before moving to the next led.
        
        for (int i = 126;i < 126+count;i++){
          byte charValue = pbuff[i];
          SoftPWMSet(pins[ledNumber], charValue);
          ledNumber++;
        }
      }
    }
  }
}
  
  static void sACNPacket(unsigned int port, byte ip[4], unsigned int i, const char *data, unsigned int len) {
  //Make sure the packet is an E1.31 packet
  //Serial.println("*");
  int count = checkACNHeaders(data, len);
  if (count){
    // It is so process the data to the LEDS
    //Serial.println(count);
    sacnDMXReceived(data, count);
  }
}

void initTest() //runs at board boot to make sure pixels are working
{
  for(int i=0;i < 255; i++){
    for(int j=0; j < CHANNEL_COUNT; j++){
      SoftPWMSet(pins[j], i);
    }
    delay(4);
  }
  delay(1000);
  for(int i=255;i >= 0; i--){
    for(int j=0; j < CHANNEL_COUNT; j++){
      SoftPWMSet(pins[j], i);
    }
    delay(4);
  }
}


void setup() {

SoftPWMBegin();

for(int i = 0; i < CHANNEL_COUNT; i++){
  SoftPWMSet(pins[i], 0);
}

initTest();

// ************************************************** ******

//Serial.begin(115200);
//Serial.println("Init.");
// No checks in here to ensure Ethernet initiated properly.
// Make sure Ethernet cable is connected and there is DHCP on the network if you are using it ....
if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0){
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


//Once the Ethernet is initialised, run a test on the LEDs
// If you have problems with lockups/reboots, you might want to disable this
//initTest();

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
