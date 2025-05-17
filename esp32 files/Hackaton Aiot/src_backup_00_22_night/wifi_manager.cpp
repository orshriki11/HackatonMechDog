#include "wifi_manager.hpp"
#include <Arduino.h>

void WiFiManager::setupLEDs() {
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
}

bool WiFiManager::begin() {
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Try to connect
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection with timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Connected to %s\n", WIFI_SSID);
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println("Failed to connect to WiFi");
        return false;
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::sendWavToServer(uint8_t* data, size_t len) {
    if (!isConnected()) {
        Serial.println("Not connected to WiFi");
        return false;
    }
    
    HTTPClient http;
    http.begin(UPLOAD_URL);
    http.addHeader("Content-Type", "application/octet-stream");
    
    Serial.printf("Sending %d bytes to %s\n", len, UPLOAD_URL);
    int httpCode = http.POST(data, len);
    
    if (httpCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Server response: " + payload);
            http.end();
            return true;
        }
    } else {
        Serial.printf("HTTP request failed: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return false;
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