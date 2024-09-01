#pragma once

#define PIN_ENCODER_A   2
#define PIN_ENCODER_B   3
#define PIN_ENCODER_BTN 11

#define PIN_LCD_CS        9
#define PIN_LCD_WR        8
#define PIN_LCD_DATA      7
#define PIN_LCD_BACKLIGHT 6

/*
                            *** Schematic ***
                              
                          +--------Uno--------+ 
                          |                   |
                          |               D11 +--[BTN]
         +-Display--+     |                D9 +--[CS]
         |   <1>    |     |                D8 +--[WR]
  [CS]---+ CS       |     |                D7 +--[DATA]
  [WR]---+ WR       |     |                D6 +--[B/L]
  [DATA]-+ DATA     |     |                   |
  [GND]--+ Gnd      |     |                   |         +---Encoder---+
  [5VDC]-+ VDD      |     |                   |         |     <2>     |
  [B/L]--+ LED+     |     |                   |         |             |
         +----------+     |                D3 +---------+ ENC_B       |
                          |                   |  [GND]--+ Gnd   Button+--[BTN]
                  [5VDC]--+ 5V             D2 +---------+ ENC_A    Gnd+--[GND]
                   [GND]--+ Gnd               |         +-------------+
                          +-------------------+ 
                                  
   
   Notes:
   [Labels] are mutually connected.
   <1>: Display is "DM8BA10", 10 char, 16 segment, blue
   <2>: Rotary encoder with push-button: https://www.jaycar.co.nz/rotary-encoder-with-pushbutton/p/SR1230
*/
