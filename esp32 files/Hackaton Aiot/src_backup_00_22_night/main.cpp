#include <Arduino.h>
#include "wifi_manager.hpp"
#include "i2s_audio.hpp"
#include "config.hpp"

// Global objects
I2SAudio audio;

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n🟢 ESP32-S3 Audio Client Starting...");
    delay(1000);
    
    // Initialize I2S
    Serial.println("📢 Initializing I2S...");
    if (!audio.begin()) {
        Serial.println("❌ I2S initialization failed");
        while (1) {
            delay(1000);
            Serial.println("Retrying I2S initialization...");
            if (audio.begin()) {
                Serial.println("✅ I2S initialization successful");
                break;
            }
        }
    }
    Serial.println("✅ I2S initialized successfully");
    
    // Connect to WiFi
    Serial.println("📡 Connecting to WiFi...");
    if (!WiFiManager::begin()) {
        Serial.println("❌ WiFi connection failed");
        while (1) {
            delay(1000);
            Serial.println("Retrying WiFi connection...");
            if (WiFiManager::begin()) {
                Serial.println("✅ WiFi connected successfully");
                break;
            }
        }
    }
    Serial.println("✅ WiFi connected successfully");
    
    // Setup I2S for microphone and speaker
    Serial.println("🎙️ Setting up microphone...");
    audio.setupMic();
    Serial.println("✅ Microphone setup complete");
    
    Serial.println("🔊 Setting up speaker...");
    audio.setupSpeaker();
    Serial.println("✅ Speaker setup complete");
    
    Serial.println("✅ Setup complete! Starting main loop...");
}

void loop() {
    static unsigned long lastRecordingTime = 0;
    const unsigned long RECORDING_INTERVAL = 5000; // Record every 5 seconds
    
    // Check if it's time to record
    if (millis() - lastRecordingTime >= RECORDING_INTERVAL) {
        Serial.println("\n🔄 Starting new recording cycle...");
        
        // Check WiFi connection
        if (!WiFiManager::isConnected()) {
            Serial.println("❌ WiFi disconnected, attempting to reconnect...");
            if (!WiFiManager::begin()) {
                Serial.println("❌ WiFi reconnection failed");
                return;
            }
            Serial.println("✅ WiFi reconnected successfully");
        }
        
        // Record audio
        Serial.println("🎙️ Recording audio...");
        audio.recordAudio();
        Serial.println("✅ Audio recorded");
        
        // Send audio to server
        Serial.println("📡 Sending audio to server...");
        if (WiFiManager::sendWavToServer(audio.getAudioBuffer(), audio.getCurrentOffset())) {
            Serial.println("✅ Audio sent successfully");
            
            // Fetch and play response
            Serial.println("⬇️ Fetching audio from server...");
            audio.fetchAndPlay();
            Serial.println("✅ Audio playback complete");
        } else {
            Serial.println("❌ Failed to send audio");
        }
        
        lastRecordingTime = millis();
        Serial.println("⏳ Waiting for next recording cycle...");
    }
    
    delay(100); // Small delay to prevent overwhelming the system
}