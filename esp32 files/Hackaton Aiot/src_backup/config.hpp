#pragma once

// WiFi + server
extern const char* ssid;
extern const char* password;
extern const char* serverUrl;

// LEDs
#define BLUE_LED_PIN 1
#define RED_LED_PIN  47

// I2S pins for INMP441
#define I2S_WS 41
#define I2S_SD 39
#define I2S_SCK 40
#define I2S_PORT I2S_NUM_0

// Audio parameters
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define CHANNEL_FORMAT I2S_CHANNEL_FMT_ONLY_LEFT
#define CHANNELS 1

// Buffer settings
#define MAX_AUDIO_BYTES 96000
#define WAV_HEADER_SIZE 44
#define CHUNK_SIZE 512
#define MAX_RECORD_TIME_SEC 3

// // I2S pins & ports
// #define I2S_BCK     14
// #define I2S_WS      35
// #define I2S_SD_MIC  38
// #define I2S_SD_SPK  41
// #define I2S_MIC_NUM I2S_NUM_0
// #define I2S_SPK_NUM I2S_NUM_1

// // Audio params
// #define SAMPLE_RATE    16000
// #define RECORD_SEC        1
// #define CHANNELS          2
// #define CHUNK_BYTES     1024           // 1 KiB chunks 