#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

constexpr uint8_t LED_PIN{47};
constexpr uint8_t BTN_PIN{7};
constexpr uint16_t STACK_SIZE{4096};
constexpr uint32_t DEBOUNCE_MS{50};

static SemaphoreHandle_t serialMutex{nullptr};
static QueueHandle_t eventQueue{nullptr};
static SemaphoreHandle_t ledSemaphor{nullptr};

enum class ButtonEvent {
    Click
};

void buttonTask(void*)
{
    auto lastState = static_cast<bool>(digitalRead(BTN_PIN));
    auto stableState = lastState;
    uint32_t lastDebounceTime = millis();

    for (;;) {
        const bool currentState = static_cast<bool>(digitalRead(BTN_PIN));

        if (currentState != lastState) {
            lastDebounceTime = millis();
            lastState = currentState;
        }

        if (millis() - lastDebounceTime >= DEBOUNCE_MS && currentState != stableState) {
            stableState = currentState;
            if (stableState) {
                const auto event{ButtonEvent::Click};
                xQueueSend(eventQueue, &event, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void eventTask(void*)
{
    ButtonEvent event;
    for (;;) {
        if (xQueueReceive(eventQueue, &event, portMAX_DELAY) == pdPASS) {
            static uint32_t lastDebounceTime = 0;

            if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
                Serial.println("Clicked");
                xSemaphoreGive(serialMutex);
            }

            xSemaphoreGive(ledSemaphor);
        }
    }
}

void ledTask(void*)
{
    auto ledState {false};
    for (;;) {
        if (xSemaphoreTake(ledSemaphor, portMAX_DELAY) == pdTRUE) {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
            
            if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
                Serial.printf("Led status: %d\n", ledState);
                xSemaphoreGive(serialMutex);
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);

    eventQueue = xQueueCreate(5, sizeof(ButtonEvent));
    if (!eventQueue) {
        Serial.println("Failed to create queue");
        exit(-1);
    }

    ledSemaphor = xSemaphoreCreateCounting(5, 0);
    if (!ledSemaphor) {
        Serial.println("Failed to create binary semaphor");
        exit(-1);
    }

    serialMutex = xSemaphoreCreateMutex();
    if (!serialMutex) {
        Serial.println("Failed to create mutex");
        exit(-1);
    }

    xTaskCreatePinnedToCore(ledTask, "ledTask", STACK_SIZE, nullptr, 1, nullptr, 0);
    xTaskCreate(buttonTask, "buttonTask", STACK_SIZE, nullptr, 1, nullptr);
    xTaskCreate(eventTask, "eventTask", STACK_SIZE, nullptr, 1, nullptr);

}

void loop()
{
}