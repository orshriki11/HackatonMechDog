#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "config.hpp"

class WiFiManager {
public:
    static bool begin();
    static bool isConnected();
    static bool sendWavToServer(uint8_t* data, size_t len);
    static void setupLEDs();
    static void updateStatus();
}; 