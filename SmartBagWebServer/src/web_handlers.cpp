#include "web_handlers.h"
#include <FastLED.h>

extern CRGB leds[];

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
      // Speed 1 = delay 2 (slowest), Speed 2 = delay 1, Speed 3 = delay 0 (fastest)
      update_delay = 3 - speed;
    }
  }
  Serial.printf("Text speed set to: %d (delay: %d)\n", server.arg("speed").toInt(), update_delay);
  server.send(200, "text/plain", "Text speed updated");
}
