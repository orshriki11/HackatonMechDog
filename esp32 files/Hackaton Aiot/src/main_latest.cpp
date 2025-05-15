#include <Arduino.h>
#include "wifi_manager.hpp"
#include "i2s_audio.hpp"
#include "config.hpp"

void setup() {
    Serial.begin(115200);
    
    // Initialize WiFi
    if (!WiFiManager::begin()) {
        Serial.println("Failed to initialize WiFi!");
        return;
    }
    
    // Initialize I2S audio
    if (!I2SAudio::begin()) {
        Serial.println("Failed to initialize I2S audio!");
        return;
    }
}

void loop() {
    Serial.println("\n=== NEW CYCLE ===");
    
    // Update WiFi status LED
    WiFiManager::updateStatus();
    
    // Only proceed if WiFi is connected
    if (WiFiManager::isConnected()) {
        if (I2SAudio::recordAndSend()) {
            delay(500);              // give server time
            I2SAudio::fetchAndPlay(); // up to 5s worth (server will cut)
        }
    }
    
    delay(2000);  // before next cycle
} 