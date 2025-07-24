#include <ESP8266WiFi.h>
#include <FastLED.h>

// FastLED configuration
#define LED_PIN     D1
#define NUM_LEDS    256
#define BRIGHTNESS  10
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define MAX_CURRENT_MILLIAMPS 2000  // Limit to 2 Amps
#define VOLTS 5


// Global variables
uint8_t hue = 0;
int currentLed = 0;
int cycle = 0;
CRGB leds[NUM_LEDS];
CRGB color = CRGB::Aquamarine;

void setup()
{
  Serial.begin(115200);
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_CURRENT_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS);

  // Clear all LEDs initially
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop()
{
  EVERY_N_MILLISECONDS(40) {
    if (currentLed < NUM_LEDS) {
          leds[currentLed] = color;
          FastLED.show();
          currentLed++;
    }
    Serial.print("SNAKE");
  }
}

/* - try EVERY_N_MILLISECONDS(40) to create a smooth animation.
   This will update the LEDs every 40 milliseconds, allowing for a fluid transition.
   You can adjust the timing to speed up or slow down the animation.
   
   - try fill_rainbow(leds, NUM_LEDS, hue, 1) from FastLED library to create a rainbow effect.
   This will fill the LEDs with a rainbow pattern, cycling through hues.
   You can adjust the hue increment to change the speed of the rainbow effect.
 */