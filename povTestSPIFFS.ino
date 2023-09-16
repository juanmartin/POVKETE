//simple poi scroll
//fastled 8х8matrix demo
//Yaroslaw Turbin 26.10.2020
//https://vk.com/ldirko
//https://www.reddit.com/user/ldirko/

#include "FastLED.h"
#include <JPEGDecoder.h>
#include "SPIFFS.h"


// Matrix size
#define NUM_ROWS 60
#define NUM_COLS 1

// Picture size
#define PIX_ROWS 60
#define PIX_COLS 60

// LEDs pin
#define DATA_PIN 4
// LED brightness
#define BRIGHTNESS 20
#define NUM_LEDS NUM_ROWS * NUM_COLS

// Define the array of leds
CRGB leds[NUM_LEDS];


// DEFINE ITERATOR Y CANTIDAD DE IMAGENES
#define CANTIMAGENES 5
int currentPic = 0;

#define DEBOUNCE_DELAY 5000 // Adjust the debounce delay in milliseconds


CRGB image[PIX_COLS * PIX_ROWS];

const char* rgbPaths[] = {"/1.rgb", "/2.rgb", "/3.rgb", "/4.rgb", "/5.rgb"};
File rgbFiles[CANTIMAGENES];
CRGB images[CANTIMAGENES][PIX_COLS * PIX_ROWS];

byte cThreshold = 1;

int hueShift = 0;
int rShift = 0, gShift = 0, bShift = 0;

int touchBackground;
bool showBackground = false;

int touchBrightness;
int brightness = BRIGHTNESS;

int touchStutter;
bool doStutter = false;


void setup() {
  // Initialize SPIFFS (on ESP32)
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  for (int i = 0; i < CANTIMAGENES; i ++) {
    rgbFiles[i] = SPIFFS.open(rgbPaths[i], "r");
    if (!rgbFiles[i]) {
      Serial.println("Failed to open BIN file");
      return;
    }
  }


  // Read binary data and populate the CRGB array
  for (int imgIdx = 0; imgIdx < CANTIMAGENES; imgIdx++) {
    for (int i = 0; i < PIX_ROWS * PIX_COLS; i++) {
      uint32_t pixelData;
      byte rData;
      rgbFiles[imgIdx].readBytes((char*)&rData, sizeof(byte));

      byte gData;
      rgbFiles[imgIdx].readBytes((char*)&gData, sizeof(byte));

      byte bData;
      rgbFiles[imgIdx].readBytes((char*)&bData, sizeof(byte));

      images[imgIdx][i] = CRGB(rData, gData, bData);


    }
  }




  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
}

void loop() {
  FastLED.setBrightness(brightness);

  // BUTTON INTERACTION
  touchBackground = touchRead(T3);
  touchBrightness = touchRead(T4);
  touchStutter = touchRead(T6);
  Serial.println(touchStutter);

  EVERY_N_MILLIS(200) {

    // SHOW BACK?
    if (touchBackground < 20) {
      showBackground = !showBackground;
    }

    // DECREMENT BRIGHTNESS
    if (touchBrightness < 20) {
      brightness -= 1;
      if (brightness < 0) {
        brightness = BRIGHTNESS;
      }
    }

    // STUTTER
    if (touchStutter < 20) {
      doStutter = !doStutter;
    }
    
  }

  // PIC CHANGE 
  EVERY_N_MILLIS(DEBOUNCE_DELAY) {
    currentPic = (currentPic + 1) % CANTIMAGENES;
  }
  
  // COLOR SHIFT
  EVERY_N_MILLIS(274) {
    rShift = (rShift + 1) % 256;
  }

  EVERY_N_MILLIS(13) {
    gShift = (gShift + 1) % 256;
  }

  EVERY_N_MILLIS(467) {
    bShift = (bShift + 1) % 256;
  }

  //STUTTER
  if (doStutter) {
    if (random(10) > 8) {
      prntpict(PIX_COLS, images[currentPic]);
    }
  } else {
    prntpict(PIX_COLS, images[currentPic]);
  }


  FastLED.show();
  //  FastLED.delay(50);
}

uint16_t XY (uint8_t x, uint8_t y) {
  return (y * NUM_COLS + x);
}

void prntpict (int len, CRGB pict[] ) {
  static uint16_t xoffs;
  static uint16_t speed = 100;
  CRGB col;

  EVERY_N_MILLISECONDS(1) {
    speed ++; //80 - speed of effect
  }

  xoffs = (speed) % (len);
  for (byte j = 0; j < NUM_ROWS; j++) {
    col = pict [(j * len) + xoffs];
    leds[j] = col;
    if (leds[j]) {
      leds[j].g += gShift;
      leds[j].b += bShift;
      leds[j].r += rShift;
    } else {
      if (showBackground) {
        leds[j].b += bShift;
        leds[j].r += rShift;
        leds[j].g += gShift;
        leds[j] /= 10;
      }

    }

  }





}
