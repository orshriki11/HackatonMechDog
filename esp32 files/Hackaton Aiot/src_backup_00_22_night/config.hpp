#pragma once

// WiFi Configuration
#define WIFI_SSID "Mechdog"  // Replace with your WiFi network name
#define WIFI_PASSWORD "Mechdog1!"       // Replace with your WiFi password (if any)
#define SERVER_URL "http://132.69.207.21:12345"
#define UPLOAD_URL "http://132.69.207.21:12345/upload"
#define SOUND_URL "http://132.69.207.21:12345/sound"

// I2S Configuration for ESP32-S3
#define I2S_BCK_PIN 40  // BCLK
#define I2S_WS_PIN  41  // LRCL or WS
#define I2S_SD_PIN  39  // SD

// Audio Configuration
#define SAMPLE_RATE 16000  // 16kHz
#define MAX_RECORD_TIME_SEC 3
#define RECORD_SEC 3  // Recording duration in seconds
#define MAX_AUDIO_BYTES (SAMPLE_RATE * MAX_RECORD_TIME_SEC * 2)  // 16-bit samples
#define WAV_HEADER_SIZE 44
#define CHUNK_SIZE 512
#define CHANNELS 1  // Mono audio

// LEDs
#define BLUE_LED_PIN 35  // Built-in blue LED on ESP32-S3-DevKitC-1
#define RED_LED_PIN  36  // Built-in red LED on ESP32-S3-DevKitC-1

// I2S Configuration
#define I2S_PORT I2S_NUM_0
#define I2S_MIC_NUM I2S_NUM_0
#define I2S_SPK_NUM I2S_NUM_1
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT  // 16-bit samples
#define CHANNEL_FORMAT I2S_CHANNEL_FMT_ONLY_LEFT

// I2S pins & ports
#define I2S_BCK     40
#define I2S_WS      41
#define I2S_SD_MIC  39
#define I2S_SD_SPK  41
// #define I2S_SPK_NUM I2S_NUM_1

// // Audio params
// #define SAMPLE_RATE    16000
// #define RECORD_SEC        1
// #define CHANNELS          2
// #define CHUNK_BYTES     1024           // 1 KiB chunks 