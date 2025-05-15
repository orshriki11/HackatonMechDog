#include <Arduino.h>
#include "led_functions.hpp"

void setupLEDs() {
    pinMode(BLUE_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, LOW);
}

void turnOnBlueLED() {
    digitalWrite(BLUE_LED, HIGH);
}

void turnOffBlueLED() {
    digitalWrite(BLUE_LED, LOW);
}

void blinkRedLED(int times, int delay_ms) {
    for (int i = 0; i < times; ++i) {
        digitalWrite(RED_LED, HIGH);
        delay(delay_ms);
        digitalWrite(RED_LED, LOW);
        delay(delay_ms);
    }
}
