#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <cstdint>
#include "bme280.h"

#define TFT_CS        6
#define TFT_DC        7
#define TFT_RES       10
#define TFT_SCK       12
#define TFT_MOSI      11

namespace {
  constexpr uint16_t  Display_Color_Red {0xF800};
  constexpr uint16_t  Display_Color_Green {0x07E0};
  constexpr uint16_t  Display_Color_Yellow{0xFFE0};
  constexpr uint16_t  Display_Color_Blue{0x001F};

  constexpr uint16_t COLOR_TEXT {0xFFFF};
  constexpr uint32_t UPDATE_INTERVAL_MS {1500};

  constexpr int16_t TEXT_X {10};
  constexpr int16_t TEXT_Y {20};
  constexpr uint16_t TEXT_WIDTH {140};
  constexpr uint16_t TEXT_HEIGHT {12};
}

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RES);
BmeTemperature bmeTemp;

uint32_t lastTimeUpdata {0};


void setupDisplay()
{
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);

  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(Display_Color_Blue);
  tft.setRotation(3);

  tft.setTextSize(1);
  tft.cp437(true);
}

void drawTemperature(float temperature)
{
  tft.fillRect(
    TEXT_X, TEXT_Y,
    TEXT_WIDTH, TEXT_HEIGHT,
    Display_Color_Blue
  );

  tft.setCursor(TEXT_X, TEXT_Y);
  tft.print("Temperature: ");
  tft.print(temperature, 2);
  tft.write(0xF8);
  tft.print("C");
}

void setup() {
  Serial.begin(115200);
  
  setupDisplay();

  tft.cp437(true);
  if (!bmeTemp.setup()) {
    while(1) {
      delay(1000);
    }
  }
}

void loop() {
  const auto time = millis();
  if (time - lastTimeUpdata >= UPDATE_INTERVAL_MS) {
    lastTimeUpdata = time;
    bmeTemp.loop();
    drawTemperature(bmeTemp.getTemperaturetFloat());
  }
}
