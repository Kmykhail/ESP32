#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

constexpr uint8_t ADC_PIN{8};
constexpr uint16_t PRODUCER_DELAY_MS{500};
constexpr uint16_t CONSUMER_DELAY_MS{300};

struct AdcSample {
  uint32_t sequence;
  uint16_t raw;
};

static QueueHandle_t adcQueue{nullptr};

void adcProducer(void *) {
  auto sequence {0u};
  for (;;) {
    AdcSample sample{
      sequence++, static_cast<uint16_t>(analogRead(ADC_PIN))
    };

    xQueueSend(adcQueue, &sample, 0);

    vTaskDelay(pdMS_TO_TICKS(PRODUCER_DELAY_MS));
  }
}

void adcConsumer(void *) {
  AdcSample received;
  for (;;) {
    if (xQueueReceive(adcQueue, &received, portMAX_DELAY) == pdPASS) {
      Serial.printf("Consumer, sequence: %i, raw: %i\n", received.sequence, received.raw);
      vTaskDelay(pdMS_TO_TICKS(CONSUMER_DELAY_MS));
    }
  }
}

void setup() {
  Serial.begin(115200);

  adcQueue = xQueueCreate(5, sizeof(AdcSample));

  if (!adcQueue) {
    Serial.println("Failed to create queue");
    exit(-1);
  }

  xTaskCreate(adcProducer, "adcProducer", 4096, nullptr, 1, nullptr);
  xTaskCreate(adcConsumer, "adcConsumer", 4096, nullptr, 1, nullptr);
}

void loop() {
}
