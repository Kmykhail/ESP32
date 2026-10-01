#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

constexpr uint8_t LED_PIN{47};
constexpr uint8_t BTN_PIN{7};

static SemaphoreHandle_t buttonSem{nullptr};

void buttonTask(void*) {
  auto previous {true};
  for (;;) {
    const auto current = static_cast<bool>(digitalRead(BTN_PIN));
    if (previous && !current) {
      xSemaphoreGive(buttonSem);
      vTaskDelay(pdMS_TO_TICKS(30));
    }

    previous = current;

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void ledTask(void*) {
  auto cnt = 0UL;
  auto ledState{false};
  for (;;) {
    if (xSemaphoreTake(buttonSem, portMAX_DELAY) == pdTRUE) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      Serial.printf("Click counter: %lu\n", ++cnt);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  buttonSem = xSemaphoreCreateBinary();
  if (!buttonSem) {
    Serial.printf("Failed to create semaphore");
    exit(-1);
  }

  xTaskCreate(buttonTask, "buttonTask", 4096, nullptr, 1, nullptr);
  xTaskCreate(ledTask, "ledTask", 4096, nullptr, 1, nullptr);
}

void loop() {
}
