/********************************************************************************************
*  KS0454 keyestudio round display meter demo with Teensy 3.2
*  based on https://www.instructables.com/Arduino-analogue-ring-meter-on-colour-TFT-display/
*  see also https://wiki.keyestudio.com/KS0454_keyestudio_Circular_TFT_LCD_Smart_Watch_Display_Module
********************************************************************************************/

#include "DFRobot_ST7687S_Latch.h"
#include <SPI.h>

// Meter colour schemes
#define RED2RED     0
#define GREEN2GREEN 1
#define BLUE2BLUE   2
#define BLUE2RED    3
#define GREEN2RED   4
#define RED2GREEN   5

#define DEG2RAD     0.0174532925

// Teensy 3.2 pins
uint8_t pin_can_tx = 3, pin_can_rx = 4, pin_lcd_cs = 10, pin_lcd_rs = 5, pin_lcd_wr = 6, pin_lcd_lck = 7; // Teensy with can
uint8_t pin_spi_ss = 10, pin_spi_mosi = 11, pin_spi_miso = 12, pin_spi_ck = 13;                           // Teensy SPI

DFRobot_ST7687S_Latch tft(pin_lcd_cs, pin_lcd_rs, pin_lcd_wr, pin_lcd_lck);


const int x = 0, y = 0, r = 64, w = 16; // coords of centre of ring, radius, w = r/4
const int vmin = 0, vmax = 6000;        // range
const int angle = 135;                  // Half the sweep angle of meter (270 degrees total)
const int seg = 5;                      // Segments are 5 degrees wide
const int inc = 10;                     // Draw segments every 10 degrees
int colour = 0;                         // Choose colour from scheme
int text_colour = 0;                    // To hold the text colour
int reading = 0;                        // Value to be displayed
int d = 0;                              // Variable used for the sinewave test waveform

/********************************************************************************************
*  Setup - run once
********************************************************************************************/
void setup(void) {
  
  tft.begin();
  tft.fillScreen(DISPLAY_BLACK);
  tft.setTextColor(DISPLAY_WHITE);
  tft.setTextBackground(DISPLAY_BLACK);
  tft.setTextSize(1); 
  tft.setCursor(38, 100);  //set text position to bottom center
  tft.print("RPM x 1000");

  d = 270;  // start from sin(270) = -1

}

/********************************************************************************************
*  Main Loop - run forever
********************************************************************************************/
void loop() {

    // Test with a slowly changing value from a Sine function
    d += 10; if (d >= 360) d = 0;

    // Test with Sine wave function, normally reading will be from a sensor
    reading = 3000 + 3000 * sin(d * DEG2RAD);

    // Draw analogue meter large, 1200 is value2 numeric
    ringMeter(reading, GREEN2RED);

}

/********************************************************************************************
*  Draw the meter on the screen
********************************************************************************************/
void ringMeter(int value1, byte scheme) {
  
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  int v = map(value1, vmin, vmax, -angle, angle); // Map the value to an angle v

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    switch (scheme) {
      case RED2RED:     colour = DISPLAY_RED; break;                             // Fixed colour
      case GREEN2GREEN: colour = DISPLAY_GREEN; break;                           // Fixed colour
      case BLUE2BLUE:   colour = DISPLAY_BLUE; break;                            // Fixed colour
      case BLUE2RED:    colour = rainbow(map(i, -angle, angle, 0, 127)); break;  // Full spectrum blue to red
      case GREEN2RED:   colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case RED2GREEN:   colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default:          colour = DISPLAY_BLUE; break;                            // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx1 = cos((i - 90) * DEG2RAD);
    float sy1 = sin((i - 90) * DEG2RAD);
    int x0 = sx1 * 48;
    int y0 = sy1 * 48;
    int x1 = sx1 * r;
    int y1 = sy1 * r;
    
    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * 48;
    int y2 = sy2 * 48;
    int x3 = sx2 * r;
    int y3 = sy2 * r;    

    if (i < v) { 
      // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else {
      // Fill in blank segments
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, DISPLAY_BLACK);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, DISPLAY_BLACK);
    }
  }

  // Convert value to a string
  char buf[4];
  //int len = 4; if (value > 999) len = 5;
  //dtostrf(value, len, 0, buf);
  dtostrf(value1/1000.0F, 3, 1, buf);
  // Set the text colour to default
  //tft.setTextBackground(DISPLAY_BLACK);
  // Print value1
  tft.setTextSize(2); //2 * text size, default text size: 6 * 8
  tft.setCursor(x+48, y+45);  //set text position to center
  // set the text colour to the last segment value
  tft.setTextColor(text_colour);
  tft.print(buf);

}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value) {
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  int red = 0; // Red is the top 5 bits of a 16 bit colour value
  int green = 0;// Green is the middle 6 bits
  int blue = 0; // Blue is the bottom 5 bits

  int quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// End Of File
// #########################################################################
