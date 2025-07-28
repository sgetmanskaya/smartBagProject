#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include "fonts.h"
#include "gift_01a_pixels.h"

// Network credentials
const char* ssid = "SmartBag";
const char* password = "12345678";

// LED configuration
#define NUM_LEDS 256
#define DATA_PIN D1
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 50
#define VOLTS 5
#define MAX_CURRENT_MILLIAMPS 2000

// Matrix configuration
#define WIDTH 16
#define HEIGHT 16
#define LET_WIDTH 5
#define LET_HEIGHT 7
#define SPACE 1

CRGB leds[NUM_LEDS];
ESP8266WebServer server(80);

// Global variables for LED effects
uint8_t r = 0, g = 255, b = 0;
String text = "Hello";
uint8_t current_delay = 0;
uint8_t update_delay = 3;

// Function to convert x,y coordinates to LED index with 180-degree rotation
int serpentine180_index(int x, int y) {
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

// Function to display image with serpentine layout and 180-degree rotation
void display_image_serpentine_180(const Color_RGB pixel_data[]) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int image_pixel_index = y * WIDTH + x;
      int led_index = serpentine180_index(x, y);
      Color_RGB pixel = pixel_data[image_pixel_index];
      leds[led_index] = CRGB(pixel.r, pixel.g, pixel.b);
    }
  }
}

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

// Function to draw a letter on the LED matrix
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
uint8_t hue_shift = 1;
int currentLed = 0;
int16_t textOffset = WIDTH;
bool fullTextFlag = false;
bool showBigHeart = false;
uint8_t heartCounter = 0;

enum ImageType { FILL, SNAKE, RAINBOW, IMAGE_GIFT, TEXT, IMAGE, ANIMATION, HEART_ANIMATION };
enum SelectedImage { Lion, Dinosaur, Koala, Gift };
ImageType imageType = TEXT;
SelectedImage selectedImage = Lion;

// Web server handler functions
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Smart Bag LED Controller</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }";
  html += "h1 { color: white; text-align: center; margin-bottom: 30px; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px 50px 30px 30px; border-radius: 20px; box-shadow: 0 10px 30px rgba(0,0,0,0.3); }";
  html += ".input-group { margin-bottom: 25px; }";
  html += "label { display: block; margin-bottom: 8px; font-weight: bold; color: #333; }";
  html += "input, select { width: 100%; padding: 12px; border: 2px solid #ddd; border-radius: 10px; font-size: 16px; transition: border-color 0.3s; }";
  html += "input:focus, select:focus { outline: none; border-color: #667eea; }";
  html += "#colorPicker { height: 50px; cursor: pointer; }";
  html += ".button { background: linear-gradient(45deg, #4CAF50, #45a049); border: none; color: white; padding: 15px 25px; text-align: center; display: inline-block; font-size: 16px; margin: 5px; cursor: pointer; border-radius: 25px; transition: all 0.3s ease; box-shadow: 0 4px 15px rgba(0,0,0,0.2); }";
  html += ".button:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(0,0,0,0.3); }";
  html += ".controls-row { display: flex; flex-wrap: wrap; gap: 10px; justify-content: center; }";
  html += "input[type='range'] { width: 100%; height: 8px; background: #ddd; border-radius: 5px; outline: none; }";
  html += "input[type='range']::-webkit-slider-thumb { appearance: none; width: 20px; height: 20px; background: #667eea; border-radius: 50%; cursor: pointer; }";
  html += "</style></head><body>";
  html += "<h1>Smart Bag LED Controller</h1>";
  html += "<div class='container'>";
  
  html += "<div class='input-group'>";
  html += "<label for='colorPicker'>Choose Color:</label>";
  html += "<input type='color' id='colorPicker' value='#00ff00' onchange='updateColor()'>";
  html += "</div>";
  
  html += "<div class='input-group'>";
  html += "<label>Special Effects:</label>";
  html += "<div class='controls-row'>";
  html += "<button class='button' onclick='setMode(\"fill\")'>Solid Color</button>";
  html += "<button class='button' onclick='setMode(\"rainbow\")'>Rainbow</button>";
  html += "<button class='button' onclick='setMode(\"snake\")'>Snake</button>";
  html += "<button class='button' onclick='setMode(\"heart\")'>Heart</button>";
  html += "</div>";
  html += "<div class='input-group' style='margin-top: 15px;'>";
  html += "<label>Rainbow Speed: <span id='speedValue'>1</span></label>";
  html += "<input type='range' id='speedSlider' min='1' max='10' value='1' onchange='setSpeed(this.value)'>";
  html += "</div>";
  html += "</div>";
  
  html += "<div class='input-group'>";
  html += "<label for='textInput'>Scrolling Text:</label>";
  html += "<input type='text' id='textInput' placeholder='Enter your text here...' value='Hello'>";
  html += "<br><br><button class='button' onclick='setText()'>Show Text</button>";
  html += "<div style='margin-top: 15px;'>";
  html += "<label>Text Speed: <span id='textSpeedValue'>1</span></label>";
  html += "<input type='range' id='textSpeedSlider' min='1' max='3' value='1' onchange='setTextSpeed(this.value)'>";
  html += "</div>";
  html += "</div>";
  
  html += "<div class='input-group'>";
  html += "<label for='imageSelect'>Select Image:</label>";
  html += "<select id='imageSelect'>";
  html += "<option value='0'>Lion</option>";
  html += "<option value='1'>Dinosaur</option>";
  html += "<option value='2'>Koala</option>";
  html += "<option value='3'>Gift</option>";
  html += "</select>";
  html += "<br><br><button class='button' onclick='setImage()'>Show Image</button>";
  html += "</div>";
  
  html += "</div>";
  
  html += "<script>";
  html += "function updateColor() {";
  html += "const color = document.getElementById('colorPicker').value;";
  html += "const r = parseInt(color.substr(1,2), 16);";
  html += "const g = parseInt(color.substr(3,2), 16);";
  html += "const b = parseInt(color.substr(5,2), 16);";
  html += "fetch('/setColor?r=' + r + '&g=' + g + '&b=' + b);";
  html += "}";
  html += "function setMode(mode) { fetch('/setMode?mode=' + mode); }";
  html += "function setText() {";
  html += "const text = document.getElementById('textInput').value;";
  html += "if (text.trim() == '') { alert('Please enter some text!'); return; }";
  html += "fetch('/setText?text=' + encodeURIComponent(text));";
  html += "}";
  html += "function setImage() {";
  html += "const img = document.getElementById('imageSelect').value;";
  html += "fetch('/setImage?img=' + img);";
  html += "}";
  html += "function setSpeed(speed) {";
  html += "document.getElementById('speedValue').textContent = speed;";
  html += "fetch('/setSpeed?speed=' + speed);";
  html += "}";
  html += "function setTextSpeed(speed) {";
  html += "document.getElementById('textSpeedValue').textContent = speed;";
  html += "fetch('/setTextSpeed?speed=' + speed);";
  html += "}";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleSetColor() {
  r = server.arg("r").toInt();
  g = server.arg("g").toInt();
  b = server.arg("b").toInt();
  Serial.printf("Color updated: R=%d, G=%d, B=%d\n", r, g, b);
  server.send(200, "text/plain", "Color updated");
}

void handleSetMode() {
  String mode = server.arg("mode");
  Serial.println("Mode set to: " + mode);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  if (mode == "fill") {
    imageType = ImageType::FILL;
  }
  else if (mode == "rainbow") {
    imageType = ImageType::RAINBOW;
    hue = 0;
  }
  else if (mode == "snake") {
    imageType = ImageType::SNAKE;
    currentLed = 0;
  }
  else if (mode == "gift") {
    imageType = ImageType::IMAGE_GIFT;
  }
  else if (mode == "heart") {
    imageType = ImageType::HEART_ANIMATION;
    showBigHeart = false;
    heartCounter = 0;
  }
  
  server.send(200, "text/plain", "Mode updated");
}

void handleSetText() {
  text = server.arg("text");
  imageType = ImageType::TEXT;
  textOffset = WIDTH;
  fullTextFlag = false;
  Serial.println("Text set to: " + text);
  server.send(200, "text/plain", "Text updated");
}

void handleSetImage() {
  String img = server.arg("img");
  int imgIndex = img.toInt();
  
  // Set the selected image based on the index
  switch(imgIndex) {
    case 0:
      selectedImage = SelectedImage::Lion;
      break;
    case 1:
      selectedImage = SelectedImage::Dinosaur;
      break;
    case 2:
      selectedImage = SelectedImage::Koala;
      break;
    case 3:
      selectedImage = SelectedImage::Gift;
      break;
    default:
      selectedImage = SelectedImage::Lion;
      break;
  }
  
  imageType = ImageType::IMAGE;
  Serial.println("Image set to index: " + img);
  server.send(200, "text/plain", "Image updated");
}

void handleSetSpeed() {
  hue_shift = server.arg("speed").toInt();
  Serial.printf("Speed set to: %d\n", hue_shift);
  server.send(200, "text/plain", "Speed updated");
}

void handleSetTextSpeed() {
  if (server.hasArg("speed")) {
    int speed = server.arg("speed").toInt();
    if (speed >= 1 && speed <= 3) {
      // Convert speed (1-3) to delay value
      // Speed 1 = delay 4 (slowest), Speed` 2 = delay 3, Speed 3 = delay 2 (fastest)
      update_delay = 3 - speed;
    }
  }
  Serial.printf("Text speed set to: %d (delay: %d)\n", server.arg("speed").toInt(), update_delay);
  server.send(200, "text/plain", "Text speed updated");
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== Smart Bag LED Controller ===");
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTS, MAX_CURRENT_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  delay(100); // Allow time for WiFi to disconnect
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  // Initialize WiFi
  Serial.println(ssid);
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
        // Display selected image
        switch(selectedImage) {
          case SelectedImage::Lion:
            display_image_serpentine_180(lion_pixels);
            break;
          case SelectedImage::Dinosaur:
            display_image_serpentine_180(dinosaur_pixels);
            break;
          case SelectedImage::Koala:
            display_image_serpentine_180(mouse_pixels); // Using mouse image for Koala
            break;
          case SelectedImage::Gift:
            display_image_serpentine_180(gift_01a_pixels);
            break;
        }
        FastLED.show();
        break;  
        
      case ImageType::FILL:
        fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
        FastLED.show();
        break;

      case ImageType::IMAGE_GIFT:
        display_image_serpentine_180(gift_01a_pixels);
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
              drawLetter(j, text[i], textOffset + j * (LET_WIDTH + SPACE), CRGB(r, g, b));
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
        
      case ImageType::ANIMATION:
        // Additional animations can be added here
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
          display_image_serpentine_180(big_heart_pixels);
          FastLED.show();
        } else {
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(heart_pixels);
          FastLED.show();
        }
      } else {
        // Show big heart for 1 cycle (500ms)
        if (heartCounter >= 1) {
          showBigHeart = false;
          heartCounter = 0;
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(heart_pixels);
          FastLED.show();
        } else {
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          display_image_serpentine_180(big_heart_pixels);
          FastLED.show();
        }
      }
    }
  }
}
