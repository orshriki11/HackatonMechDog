#pragma once

// WiFi + server
extern const char* ssid;
extern const char* password;
extern const char* uploadURL;
extern const char* soundURL;

// LEDs
#define BLUE_LED_PIN 1
#define RED_LED_PIN  47

// I2S pins & ports
#define I2S_BCK     14
#define I2S_WS      35
#define I2S_SD_MIC  38
#define I2S_SD_SPK  41
#define I2S_MIC_NUM I2S_NUM_0
#define I2S_SPK_NUM I2S_NUM_1

// Audio params
#define SAMPLE_RATE    16000
#define RECORD_SEC        1
#define CHANNELS          2
#define CHUNK_BYTES     1024           // 1 KiB chunks 