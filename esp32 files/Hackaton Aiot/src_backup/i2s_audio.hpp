#pragma once
#include <driver/i2s.h>
#include "config.hpp"

class I2SAudio {
public:
    static bool begin();
    static void recordAudio();
    static void addWavHeader(uint8_t* wav, uint32_t pcmDataSize);
    static size_t getCurrentOffset() { return currentOffset; }
    static uint8_t* getAudioBuffer() { return audioBuffer; }
    static uint8_t* getWavBuffer() { return wavBuffer; }

private:
    static void setupI2S();
    static uint8_t audioBuffer[MAX_AUDIO_BYTES];
    static uint8_t wavBuffer[WAV_HEADER_SIZE + MAX_AUDIO_BYTES];
    static size_t currentOffset;
}; 