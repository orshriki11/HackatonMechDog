#include "wifi_manager.hpp"
#include <Arduino.h>

void WiFiManager::setupLEDs() {
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
}

bool WiFiManager::begin() {
    setupLEDs();
    
    Serial.print("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    
    Serial.println(" connected!");
    digitalWrite(BLUE_LED_PIN, HIGH);
    return true;
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::updateStatus() {
    if (isConnected()) {
        digitalWrite(BLUE_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
    } else {
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
    }
} 