/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL4LOHbfHlF"
#define BLYNK_TEMPLATE_NAME         "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "uy1WnFtZ1qgv-bpsu3CrlIWNIHKtp-R0"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <FastLED.h>
#include "gift_01a_pixels.h"
#include "fonts.h"

// WiFi credentials
char ssid[] = "Vodafone-04D0";        // Замените на имя вашей WiFi сети
char pass[] = "8EM6qJJmBBtUbCJD";       // Замените на пароль вашей WiFi сети

// FastLED configuration
#define LED_PIN     D1
#define NUM_LEDS    256
#define WIDTH       16
#define HEIGHT      16
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define MAX_CURRENT_MILLIAMPS 2000  // Limit to 2 Amps
#define VOLTS 5

CRGB leds[NUM_LEDS];
int r = 127; int g = 0; int b = 127;

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


// Global variables
uint8_t hue = 0;
int currentLed = 0;
enum ImageType { FILL, SNAKE, RAINBOW, IMAGE_GIFT, TEXT };
ImageType imageType = TEXT;

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
  COPY_GIFT_01A_IMAGE_TO_LEDS_SERPENTINE(leds, 0);
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
  fill_solid(leds, NUM_LEDS, CRGB::Black);
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
      case ImageType::FILL:
        // Fill with solid color
        fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
        FastLED.show();
        //Serial.print("FILL");
        break;

      case ImageType::IMAGE_GIFT:
        COPY_GIFT_01A_IMAGE_TO_LEDS_SERPENTINE(leds, 0);
        FastLED.show();
        //Serial.print("IMAGE_GIFT");
        break;

      case ImageType::RAINBOW:
        hue++;
        if (hue > 255) {
          hue = 0;
        }
        fill_rainbow(leds, NUM_LEDS, hue, 1);
        FastLED.show();
        //Serial.print("RAINBOW");
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

      case ImageType::TEXT:
        // Draw text effect
        drawLetter(0, 'R', 0, CRGB(r, g, b));
        FastLED.show();
        break;

      default:
        // Do nothing for unrecognized image type
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        imageType = ImageType::RAINBOW; // Default to rainbow if unrecognized
        hue = 0; // Reset hue for rainbow effect
        //Serial.print("DEFAULT_RAINBOW");
        break;
    }
  }
}