#ifndef TEXT_PROCESSING_H
#define TEXT_PROCESSING_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "fonts.h"

// Function declarations
uint16_t getPixelNumber(int8_t x, int8_t y);
uint8_t getFont(uint8_t font, uint8_t row);
void drawLetter(CRGB *leds, uint8_t index, uint8_t letter, int16_t offset, CRGB color);

// Implementation
inline uint16_t getPixelNumber(int8_t x, int8_t y) {
  if (y % 2 == 0) {
    // even row            
    return (y * WIDTH + x);
  } else {
    // odd row                                              
    return (y * WIDTH + WIDTH - x - 1);
  }
}

// font interpreter for character codes in the fontHEX array (for Arduino IDE 1.8.* and later)
inline uint8_t getFont(uint8_t font, uint8_t row) {
  // translate character code from ASCII table to array index
  font = font - '0' + 16;
  // for Latin letters and symbols
  if (font <= 90) return pgm_read_byte(&(fontHEX[font][row]));
  else if (font >= 112 && font <= 159) {
    // and for Cyrillic
    return pgm_read_byte(&(fontHEX[font - 17][row]));
  } else if (font >= 96 && font <= 111) {
    return pgm_read_byte(&(fontHEX[font + 47][row]));
  }
  return 0;
}

// Function to draw a letter on the LED matrix
inline void drawLetter(CRGB *leds, uint8_t index, uint8_t letter, int16_t offset, CRGB color) {
  int8_t start_pos = 0, finish_pos = LET_WIDTH;
  int8_t LH = LET_HEIGHT;
  if (LH > HEIGHT) LH = HEIGHT;
  // center the letter vertically
  int8_t offset_y = (HEIGHT - LH) / 2;     

  if (offset < -LET_WIDTH || offset > WIDTH) return;
  if (offset < 0) start_pos = -offset;
  if (offset > (WIDTH - LET_WIDTH)) finish_pos = WIDTH - offset;

  for (byte i = start_pos; i < finish_pos; i++) {
    int thisByte;
    if (MIRR_V) thisByte = getFont((byte)letter, LET_WIDTH - 1 - i);
    else thisByte = getFont((byte)letter, i);

    for (byte j = 0; j < LH; j++) {
      boolean thisBit;

      if (MIRR_H) thisBit = thisByte & (1 << j);
      else thisBit = thisByte & (1 << (LH - 1 - j));

      // draw a column (i - horizontal position, j - vertical)
      if (thisBit) leds[getPixelNumber(offset + i, offset_y + TEXT_HEIGHT + j)] = color;
    }
  }
}

#endif // TEXT_PROCESSING_H