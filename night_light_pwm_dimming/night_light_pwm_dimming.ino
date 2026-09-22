#include "ButtonControl.h"
#include "Led.h"
#include "Ldr.h"
#include "SerialCommandHandler.h"

constexpr uint8_t BTN_PIN  = 7;
constexpr uint8_t ADC_PIN  = 8;
constexpr uint8_t LED_PIN  = 47;

ButtonControl<BTN_PIN> btnControl;
Led<LED_PIN> led;
Ldr<ADC_PIN> ldr;
SerialCommandHandler sch;

void setup() {
  Serial.begin(115200);

  if (!led.begin()) {
    Serial.println("Initialization error, PWM ERROR!");
    while (true) { delay(1000); }
  }

  Serial.println("READY");
}

void loop() {
  sch.updata(btnControl, led, ldr);

  btnControl.update();

  if (btnControl.isOn()) {
    ldr.update();
    led.setBrightnessFromAdc(ldr.getFiltered());
  } else {
    ldr.reset();
    led.stop();
  }
}