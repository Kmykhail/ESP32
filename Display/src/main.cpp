#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "bme280.h"

#define TFT_CS        6
#define TFT_DC        7
#define TFT_RES       10
#define TFT_SCK       12
#define TFT_MOSI      11

constexpr uint16_t  Display_Color_Red {0xF800}; // yellow
constexpr uint16_t  Display_Color_Green {0x07E0}; // somekind a purple
constexpr uint16_t  Display_Color_Yellow{0xFFE0};
constexpr uint16_t  Display_Color_Blue{0x001F}; // ok

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RES);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  Serial.println("OPA");
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(Display_Color_Blue);
  tft.setRotation(3);
  // tft.cp437(true);
  // tft.print("OPA");
  // tft.print(34);
  // tft.write(0xF8); // Print the degrees symbol
}

void loop() {
}
