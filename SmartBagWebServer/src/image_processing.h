#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

typedef CRGB Color_RGB;

// Function to convert x,y coordinates to LED index with 180-degree rotation
int serpentine180_index(int x, int y);

// Function to display image with serpentine layout and 180-degree rotation
void display_image_serpentine_180(CRGB *leds, const Color_RGB pixel_data[]);

// Implementation
inline int serpentine180_index(int x, int y) {
  int rotated_x = WIDTH - 1 - x;
  int rotated_y = HEIGHT - 1 - y;
  int index;
  
  if (rotated_y % 2 == 0) {
    index = rotated_y * WIDTH + rotated_x;
  } else {
    index = rotated_y * WIDTH + (WIDTH - 1 - rotated_x);
  }
  
  return index;
}

inline void display_image_serpentine_180(CRGB *leds, const Color_RGB pixel_data[]) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int image_pixel_index = y * WIDTH + x;
      int led_index = serpentine180_index(x, y);
      Color_RGB pixel = pixel_data[image_pixel_index];
      leds[led_index] = CRGB(pixel.r, pixel.g, pixel.b);
    }
  }
}

#endif // IMAGE_PROCESSING_H