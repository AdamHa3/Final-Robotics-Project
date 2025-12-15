#include <Adafruit_NeoPixel.h>   // Library for controlling addressable RGB LEDs

// =====================================================
//                RGB LED SETTINGS
// =====================================================
#define RGB_PIN 4               // Pin connected to NeoPixel data input
#define LED_COUNT 1             // Number of NeoPixels on the robot

// Create NeoPixel object
// LED_COUNT  -> total LEDs
// RGB_PIN    -> data pin
// GRB order  -> standard NeoPixel format
Adafruit_NeoPixel rgb(LED_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

// =====================================================
//            NEOPIXEL INITIALIZATION
// =====================================================
void startRGB() {
  rgb.begin();                  // Start NeoPixel communication
  rgb.clear();                  // Clear any previous colors
  rgb.show();                   // Update LED (turns it off)
}

// =====================================================
//              RGB COLOR HELPER
// =====================================================
void setRGB(uint8_t red, uint8_t green, uint8_t blue) {
  rgb.setPixelColor(0, rgb.Color(red, green, blue)); // Set first LED color
  rgb.show();                                        // Apply the change
}

// =====================================================
//              MODE COLOR INDICATORS
// =====================================================
void rgbMode1() {               // Manual mode
  setRGB(0, 0, 255);             // Blue
}

void rgbMode2() {               // Line tracking mode
  setRGB(0, 255, 0);             // Green
}

void rgbMode3() {               // Obstacle avoidance mode
  setRGB(255, 0, 0);             // Red
}

void rgbMode4() {               // Victory lap mode
  setRGB(160, 0, 255);           // Purple (slightly different shade)
}

void rgbWarning() {             // Warning / lost-line indicator
  setRGB(255, 170, 0);           // Orange
}

