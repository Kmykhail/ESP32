#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
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
static QueueHandle_t queue{nullptr};

void setupDisplay()
{
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);

  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.fillScreen(Display_Color_Blue);
  tft.setRotation(3);

  tft.setTextSize(1);
  tft.cp437(true);
}

void drawTemperature(void *)
{
  auto temperature{0.0f};

  for (;;) {
    if (xQueueReceive(queue, &temperature, portMAX_DELAY) == pdPASS) {
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
  }
}

void temperatureReader(void *)
{
  auto temperature{0.0f};
  for (;;) {
      bmeTemp.loop();

      temperature = bmeTemp.getTemperaturetFloat();

      xQueueOverwrite(queue, &temperature);

      vTaskDelay(pdMS_TO_TICKS(UPDATE_INTERVAL_MS));
  }
}

void setup() {
  Serial.begin(115200);
  
  if (!bmeTemp.setup()) {
    while(1) {
      delay(1000);
    }
  }

  setupDisplay();
  queue = xQueueCreate(1, sizeof(float));
  if (!queue) {
    Serial.println("Failed to create queue");
    exit(-1);
  }

  xTaskCreate(temperatureReader, "temperatureReader", 4096, nullptr, 1, nullptr);
  xTaskCreate(drawTemperature, "drawTemperature", 4096, nullptr, 1, nullptr);

}

void loop() {
}
