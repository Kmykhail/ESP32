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

BmeTemperature bmeTemp;

void setup() {
  Serial.begin(115200);
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  Serial.println("OPA");
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(Display_Color_Blue);
  tft.setRotation(3);

  tft.setTextSize(1);

  tft.cp437(true);
  if (!bmeTemp.setup()) {
    exit(-1);
  }
}

void loop() {
  bmeTemp.loop();


  const auto temp = bmeTemp.getTemperaturetFloat();

  tft.fillRect(10, 20, 130, 20, Display_Color_Blue);
  tft.setCursor(10, 20);
  tft.print("Temperature: ");
  tft.print(temp, 2);
  tft.write(0xF8);
  tft.print("C");

  // Serial.printf("Temperature: %.2f C\n", bmeTemp.getTemperaturetFloat());
  delay(3000);
}
