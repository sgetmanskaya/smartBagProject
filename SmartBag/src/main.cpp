/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL4SOS80OHd"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "M15afJbKslgugjYSDqBpSjHJkbP7Ya2J"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <FastLED.h>
#include "gift_01a_pixels.h"
#include "fonts.h"

// WiFi credentials
//char ssid[] = "Vodafone-04D0";        // Замените на имя вашей WiFi сети
//char pass[] = "8EM6qJJmBBtUbCJD";       // Замените на пароль вашей WiFi сети

// WiFi credentials
char ssid[] = "iPhone Sveta";  
char pass[] = "svetusya"; 

// FastLED configuration
#define LED_PIN     D1
#define NUM_LEDS    256
#define WIDTH       16
#define HEIGHT      16
#define SPACE       1
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define MAX_CURRENT_MILLIAMPS 2000  // Limit to 2 Amps
#define VOLTS 5

CRGB leds[NUM_LEDS];
int r = 127; int g = 0; int b = 127;
uint8_t update_delay = 0;
uint8_t current_delay = 0;
bool heart_bit = false;
String text = "KGU"; // Text to scroll

#define TEXT_HEIGHT 0     // высота, на которой бежит текст (от низа матрицы)
#define LET_WIDTH 5       // ширина буквы шрифта
#define LET_HEIGHT 8      // высота буквы шрифта
#define MIRR_V 0          // отразить текст по вертикали (0 / 1)
#define MIRR_H 0          // отразить текст по горизонтали (0 / 1)

uint16_t getPixelNumber(int8_t x, int8_t y) {
  if (y % 2 == 0) {               // если чётная строка
    return (y * WIDTH + x);
  } else {                                              // если нечётная строка
    return (y * WIDTH + WIDTH - x - 1);
  }
}

// интерпретатор кода символа в массиве fontHEX (для Arduino IDE 1.8.* и выше)
uint8_t getFont(uint8_t font, uint8_t row) {
  font = font - '0' + 16;   // перевод код символа из таблицы ASCII в номер согласно нумерации массива
  if (font <= 90) return pgm_read_byte(&(fontHEX[font][row]));     // для английских букв и символов
  else if (font >= 112 && font <= 159) {    // и пизд*ц для русских
    return pgm_read_byte(&(fontHEX[font - 17][row]));
  } else if (font >= 96 && font <= 111) {
    return pgm_read_byte(&(fontHEX[font + 47][row]));
  }
  return 0;
}

void drawLetter(uint8_t index, uint8_t letter, int16_t offset, CRGB color) {
  int8_t start_pos = 0, finish_pos = LET_WIDTH;
  int8_t LH = LET_HEIGHT;
  if (LH > HEIGHT) LH = HEIGHT;
  int8_t offset_y = (HEIGHT - LH) / 2;     // по центру матрицы по высоте
  
  // CRGB letterColor;
  // if (color == 1) letterColor = CHSV(byte(offset * 10), 255, 255);
  // else if (color == 2) letterColor = CHSV(byte(index * 30), 255, 255);
  // else letterColor = color;

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

      // рисуем столбец (i - горизонтальная позиция, j - вертикальная)
      if (thisBit) leds[getPixelNumber(offset + i, offset_y + TEXT_HEIGHT + j)] = color;
    }
  }
}

void setImage(Color_RGB* image) {
  for(int row = 0; row < HEIGHT; row++) { 
    for(int col = 0; col < WIDTH; col++) { 
      int image_pixel_index = row * WIDTH + col;
      int led_pixel_index; 
      if (row % 2 == 0) { 
        /* Even rows: left to right */ 
        led_pixel_index = row * WIDTH + col; 
      } else { 
        /* Odd rows: right to left */ 
        led_pixel_index = row * WIDTH + (WIDTH - 1 - col); 
      } 
      if (image_pixel_index < NUM_LEDS) { 
        leds[led_pixel_index] = image[image_pixel_index]; 
      } 
    }
  } 
}

/**
 * Convert 2D matrix coordinates to 1D LED strip index
 * using serpentine pattern rotated 180 degrees
 * 
 * In a 180° rotated serpentine pattern:
 * - We start from bottom-right corner instead of top-left
 * - Bottom rows (when counting from original bottom): 
 *   - Even rows go right-to-left
 *   - Odd rows go left-to-right
 * 
 * @param x: X coordinate (0 to MATRIX_WIDTH-1)
 * @param y: Y coordinate (0 to MATRIX_HEIGHT-1) 
 * @return: LED strip index (0 to NUM_LEDS-1)
 */
int serpentine180_index(int x, int y) {
    // Rotate 180° by flipping only Y coordinate (vertical flip only)
    // This gives us 180° rotation without horizontal mirroring
    int rotated_x = x;  // Keep X unchanged
    int rotated_y = HEIGHT - 1 - y;  // Flip Y only
    
    // Calculate which row we're on when counting from the bottom (after rotation)
    int bottom_row = HEIGHT - 1 - rotated_y;
    
    int led_index;
    if (bottom_row % 2 == 0) {
        // Even rows from bottom: right to left
        led_index = rotated_y * WIDTH + (WIDTH - 1 - rotated_x);
    } else {
        // Odd rows from bottom: left to right  
        led_index = rotated_y * WIDTH + rotated_x;
    }
    
    return led_index;
}

/**
 * Display pixel data from header file using serpentine 180° pattern
 * Simplified for 16x16 images with no offset
 * 
 * @param pixel_data: Array of Color_RGB pixels from header file (16x16 = 256 pixels)
 */
void display_image_serpentine_180(const Color_RGB* pixel_data) {
    // Iterate through each pixel in the 16x16 image
    for (int y = 0; y < WIDTH; y++) {
        for (int x = 0; x < HEIGHT; x++) {
            // Calculate image pixel index (row-major order)
            int image_pixel_index = y * HEIGHT + x;
            
            // Get the serpentine LED index
            int led_index = serpentine180_index(x, y);
            
            // Get pixel color from header data
            Color_RGB pixel = pixel_data[image_pixel_index];
            
            // Set LED color
            leds[led_index] = CRGB(pixel.r, pixel.g, pixel.b);
        }
    }
}

// Global variables
uint8_t hue = 0;
uint8_t hue_shift = 1;
int currentLed = 0;
int16_t textOffset = WIDTH;  // Global offset for scrolling text
bool fullTextFlag = false;   // Global flag for text completion
bool showBigHeart = false;  // Flag to track which heart to show
uint8_t heartCounter = 0;   // Counter for heart animation timing
enum ImageType { FILL, SNAKE, RAINBOW, IMAGE_GIFT, TEXT, IMAGE, ANIMATION, HEART_ANIMATION };
enum SelectedImage { Item1, Item2 };
ImageType imageType = FILL;

// Blynk Virtual Pin V0 - Button for solid color
BLYNK_WRITE(V0)
{
  imageType = ImageType::FILL;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(5);
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  FastLED.show();
}

// Blynk Virtual Pin V1 - Button for image
BLYNK_WRITE(V1)
{
  imageType = ImageType::IMAGE_GIFT;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(5);
  display_image_serpentine_180(gift_01a_pixels);
  //COPY_GIFT_01A_IMAGE_TO_LEDS_SERPENTINE(leds, 0);
  FastLED.show();
}

BLYNK_WRITE(V3) {   
  // Called when virtual pin V2 is updated from the Blynk.App
  // V2 is a datastream of data type String assigned to a   
  // Blynk.App ZeRGBa widget.
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
  //fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
} // BLYNK_WRITE(V2)

// Blynk Virtual Pin V4 - Button for rainbow effect
BLYNK_WRITE(V4)
{
  imageType = ImageType::RAINBOW;
  hue = 0; // Reset hue for rainbow effect
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.delay(5);
  // Create rainbow effect
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(map(i, 0, NUM_LEDS-1, 0, 255), 255, 255);
  }
  FastLED.show();
}

// Blynk Virtual Pin V5 - Button for snake effect
BLYNK_WRITE(V5)
{
  imageType = ImageType::SNAKE;
  currentLed = 0;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// Blynk Virtual Pin V6 - Button for text effect
BLYNK_WRITE(V6)
{
  imageType = ImageType::TEXT;
  textOffset = WIDTH;  // Reset text position when text mode is activated
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

BLYNK_WRITE(V9)
{
  imageType = ImageType::TEXT;
  textOffset = WIDTH;  // Reset text position when text mode is activated
  text = param.asStr(); // Get text from Blynk app
  Serial.print(text);
  fullTextFlag = false; // Reset flag for text completion
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// Blynk Virtual Pin V7 - rainbow hue shift
BLYNK_WRITE(V7)
{
  hue_shift = param.asInt();
}

BLYNK_WRITE(V8)
{
  update_delay = 1 - param.asInt();
}

BLYNK_WRITE(V10)
{
  imageType = ImageType::IMAGE;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  int img = param.asInt();
  if (img == SelectedImage::Item1) {
    display_image_serpentine_180(orient_pixels);
    FastLED.show();
  } else if (img == SelectedImage::Item2) {
    display_image_serpentine_180(gift_01a_pixels);
    FastLED.show();
  }
}

// Blynk Virtual Pin V11 - Button for heart animation
BLYNK_WRITE(V11)
{
  imageType = ImageType::HEART_ANIMATION;
  showBigHeart = false;  // Start with small heart
  heartCounter = 0;      // Reset counter
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  display_image_serpentine_180(heart_pixels);  // Show small heart first
  FastLED.show();
}

void setup()
{
  Serial.begin(115200);
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_CURRENT_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Clear all LEDs initially
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop()
{
  // Run Blynk
  Blynk.run();

  EVERY_N_MILLISECONDS(40) {
    switch(imageType) {
      case ImageType::IMAGE:
        // Do nothing, just wait for Blynk commands
        break;  
      case ImageType::FILL:
        // Fill with solid color
        fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
        FastLED.show();
        //Serial.print("FILL");
        break;

      case ImageType::IMAGE_GIFT:
        display_image_serpentine_180(gift_01a_pixels);
        FastLED.show();
        //Serial.print("IMAGE_GIFT");
        break;

      case ImageType::RAINBOW:
        hue+= hue_shift; 
        if (hue > 255) {
          hue = 0;
        }
        fill_rainbow(leds, NUM_LEDS, hue, 1);
        FastLED.show();
        //Serial.print("RAINBOW");
        break;

      case ImageType::TEXT:
        {
          if (current_delay < update_delay) {
            current_delay++;
            break; // Skip the rest of the loop until the next update
          }
          // Draw scrolling text effect
          FastLED.clear();
          byte i = 0, j = 0;
          
          // Draw each character at its current position
          while (text[i] != '\0') {
              drawLetter(j, text[i], textOffset + j * (LET_WIDTH + SPACE), CRGB(r, g, b));
              i++;
              j++;
          }

          // Move text to the left
          textOffset--;
          
          // Reset position when text has completely scrolled off screen
          if (textOffset < -j * (LET_WIDTH + SPACE)) {
            textOffset = WIDTH + 3;
            fullTextFlag = true;
          }
          current_delay = 0; // Reset delay counter
          FastLED.show();
        }
        break;

      case ImageType::SNAKE:
        if (currentLed < NUM_LEDS) {
          leds[currentLed] = CRGB(r, g, b);
          FastLED.show();
          currentLed++;
        } else {
          currentLed = 0; // Reset for next cycle
          fill_solid(leds, NUM_LEDS, CRGB::Black); // Clear the LEDs
          FastLED.show();
        }
        //Serial.print("SNAKE");
        break;
    }
  }
  EVERY_N_MILLISECONDS(500) {
    switch(imageType) {
        case ImageType::HEART_ANIMATION:
        {
          // Heart animation timing: small heart 80ms (2 cycles), big heart 40ms (1 cycle)
          heartCounter++;
          
          if (!showBigHeart) {
            // Currently showing small heart - show for 2 cycles (80ms)
            if (heartCounter >= 2) {
              showBigHeart = true;
              heartCounter = 0;
              fill_solid(leds, NUM_LEDS, CRGB::Black);
              display_image_serpentine_180(big_heart_pixels);
              FastLED.show();
            } else {
              // Continue showing small heart
              fill_solid(leds, NUM_LEDS, CRGB::Black);
              display_image_serpentine_180(heart_pixels);
              FastLED.show();
            }
          } else {
            // Currently showing big heart - show for 1 cycle (40ms)
            if (heartCounter >= 1) {
              showBigHeart = false;
              heartCounter = 0;
              fill_solid(leds, NUM_LEDS, CRGB::Black);
              display_image_serpentine_180(heart_pixels);
              FastLED.show();
            } else {
              // Continue showing big heart
              fill_solid(leds, NUM_LEDS, CRGB::Black);
              display_image_serpentine_180(big_heart_pixels);
              FastLED.show();
            }
          }
        }
        break;
    }
  }
}