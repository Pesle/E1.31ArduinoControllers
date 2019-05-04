# E1.31 Arduino Christmas Light Controllers
This is a collection of Arduino code to run a ENC28J60 Ethernet module on an Arduino that uses the E1.31 Protocol, and outputs to different types of Christmas lights.

The main aim of this project is to be able to control Christmas lights on a budget, so the system has quite a few limitations compared to a professional controller, such as its inability to handle Multicast, and its limited outputs. I am planning on upgrading the code for a STM32 board in the future for more outputs and hopefully Multicast.

--E1.31_2W--
This code uses my own LED_2W library to use a H-Bridge to control the output
of newer 2 wire Christmas Lights that need inverting signals.

--E1.31_W--
This code uses SoftPWM to get 12 output channels which can use either
use MosFETS or transistors to output to Christmas Lights

The E1.31 Receiver code is based Andrew Huxtables code on the AusChristmasLighting Forum:
https://auschristmaslighting.com/threads/another-arduino-pixel-controller.6528/
