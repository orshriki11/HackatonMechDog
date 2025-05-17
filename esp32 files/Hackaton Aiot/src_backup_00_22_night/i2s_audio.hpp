#pragma once
#include <driver/i2s.h>
#include <Arduino.h>
#include "config.hpp"

class I2SAudio {
public:
    static bool begin();
    static void read(int32_t* samples, size_t count);
    static void stop();
    static void addWavHeader(uint8_t* wav, uint32_t pcmDataSize);
    static size_t getCurrentOffset() { return currentOffset; }
    static uint8_t* getAudioBuffer() { return audioBuffer; }
    static uint8_t* getWavBuffer() { return wavBuffer; }
    static void recordAudio();
    static void setupMic();
    static void setupSpeaker();
    static bool recordAndSend();
    static void fetchAndPlay();

private:
    static void configureI2S();
    static uint8_t audioBuffer[MAX_AUDIO_BYTES];
    static uint8_t wavBuffer[WAV_HEADER_SIZE + MAX_AUDIO_BYTES];
    static size_t currentOffset;
}; 