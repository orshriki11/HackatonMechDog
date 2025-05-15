#pragma once
#include <WiFi.h>
#include "config.hpp"

class WiFiManager {
public:
    static bool begin();
    static bool isConnected();
    static void updateStatus();
private:
    static void setupLEDs();
}; 