#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include "config.h"
#include "image_pixels.h"
#include "text_processing.h"
#include "image_processing.h"
#include "web_handlers.h"

// Network credentials
const char* ssid = "SmartBag";
const char* password = "12345678";

// LED array and server
CRGB leds[NUM_LEDS];
ESP8266WebServer server(80);

// Global variables for LED effects
uint8_t r = 0, g = 255, b = 0;
String text = "Hello";

// Animation variables
uint8_t hue = 0;
uint8_t hue_shift = 1;
int currentLed = 0;
int8_t textOffset = WIDTH;
bool fullTextFlag = false;
bool showBigHeart = false;
uint8_t heartCounter = 0;
uint8_t current_delay = 0;
uint8_t update_delay = 3;

// State variables
ImageType imageType = TEXT;
SelectedImage selectedImage = Lion;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Smart Bag LED Controller ===");
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_CURRENT_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  // Initialize WiFi
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("Open this URL in your browser to control the LEDs!");

  // Configure server routes
  server.on("/", handleRoot);
  server.on("/setColor", handleSetColor);
  server.on("/setMode", handleSetMode);
  server.on("/setText", handleSetText);
  server.on("/setImage", handleSetImage);
  server.on("/setSpeed", handleSetSpeed);
  server.on("/setTextSpeed", handleSetTextSpeed);
  
  // Start web server
  server.begin();
  Serial.println("Web server started on port 80");

  // Initial LED setup
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  Serial.println("System ready!");
}

void loop() {
  server.handleClient();
  
  // Main animation loop with 40ms delay
  EVERY_N_MILLISECONDS(40) {
    switch(imageType) {
      case ImageType::IMAGE:
        switch(selectedImage) {
          case SelectedImage::Lion:
            display_image_serpentine_180(leds, lion_pixels);
            break;
          case SelectedImage::Dinosaur:
            display_image_serpentine_180(leds, dinosaur_pixels);
            break;
          case SelectedImage::Koala:
            display_image_serpentine_180(leds, mouse_pixels); // Using mouse image for Koala
            break;
          case SelectedImage::Gift:
            display_image_serpentine_180(leds, gift_01a_pixels);
            break;
        }
        FastLED.show();
        break;  
        
      case ImageType::FILL:
        fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
        FastLED.show();
        break;

      case ImageType::IMAGE_GIFT:
        display_image_serpentine_180(leds, gift_01a_pixels);
        FastLED.show();
        break;

      case ImageType::RAINBOW:
        hue += hue_shift; 
        if (hue > 255) {
          hue = 0;
        }
        fill_rainbow(leds, NUM_LEDS, hue, 1);
        FastLED.show();
        break;

      case ImageType::TEXT:
        {
          if (current_delay < update_delay) {
            current_delay++;
            break;
          }
          
          FastLED.clear();
          byte i = 0, j = 0;
          
          while (text[i] != '\0') {
              drawLetter(leds, j, text[i], textOffset + j * (LET_WIDTH + SPACE), CRGB(r, g, b));
              i++;
              j++;
          }

          textOffset--;
          
          if (textOffset < -j * (LET_WIDTH + SPACE)) {
            textOffset = WIDTH + 3;
            fullTextFlag = true;
          }
          current_delay = 0;
          FastLED.show();
        }
        break;

      case ImageType::SNAKE:
        if (currentLed < NUM_LEDS) {
          leds[currentLed] = CRGB(r, g, b);
          FastLED.show();
          currentLed++;
        } else {
          currentLed = 0;
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          FastLED.show();
        }
        break;
        
      case ImageType::HEART_ANIMATION:
        // Heart animation handled in separate timer below
        break;
    }
  }

  // Heart animation with 2:1 timing ratio (400ms intervals)
  EVERY_N_MILLISECONDS(400) {
    if (imageType == ImageType::HEART_ANIMATION) {
      heartCounter++;
      
      if (!showBigHeart) {
        // Show small heart for 2 cycles (800ms total)
        if (heartCounter >= 2) {
          showBigHeart = true;
          heartCounter = 0;
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(leds, big_heart_pixels);
          FastLED.show();
        } else {
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(leds, heart_pixels);
          FastLED.show();
        }
      } else {
        // Show big heart for 1 cycle (400ms)
        if (heartCounter >= 1) {
          showBigHeart = false;
          heartCounter = 0;
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(leds, heart_pixels);
          FastLED.show();
        } else {
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(leds, big_heart_pixels);
          FastLED.show();
        }
      }
    }
  }
}
