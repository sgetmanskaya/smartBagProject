#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <ESP8266WebServer.h>
#include "config.h"

// External references to global variables
extern ESP8266WebServer server;
extern uint8_t r, g, b;
extern String text;
extern ImageType imageType;
extern SelectedImage selectedImage;
extern uint8_t update_delay;
extern uint8_t hue;
extern uint8_t hue_shift;
extern int currentLed;
extern int8_t textOffset;
extern bool fullTextFlag;
extern bool showBigHeart;
extern uint8_t heartCounter;

// Web server handler function declarations
void handleRoot();
void handleSetColor();
void handleSetMode();
void handleSetText();
void handleSetImage();
void handleSetSpeed();
void handleSetTextSpeed();

#endif // WEB_HANDLERS_H
