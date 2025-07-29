#ifndef CONFIG_H
#define CONFIG_H

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

// Text configuration
#define LET_WIDTH 5
#define LET_HEIGHT 7
#define SPACE 1
#define TEXT_HEIGHT 0
#define MIRR_V 0
#define MIRR_H 0

// Network credentials
extern const char* ssid;
extern const char* password;

// Enums
enum ImageType { FILL, SNAKE, RAINBOW, IMAGE_GIFT, TEXT, IMAGE, ANIMATION, HEART_ANIMATION };
enum SelectedImage { Lion, Dinosaur, Koala, Gift };

#endif // CONFIG_H
