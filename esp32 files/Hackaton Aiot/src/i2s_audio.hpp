#pragma once
#include <HTTPClient.h>
#include "driver/i2s.h"
#include "config.hpp"

class I2SAudio {
public:
    static bool begin();
    static bool recordAndSend();
    static void fetchAndPlay();
private:
    static void setupMic();
    static void setupSpeaker();
}; 