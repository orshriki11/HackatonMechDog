// #include <Arduino.h>
// #include "wifi_manager.hpp"
// #include "i2s_audio.hpp"
// #include "config.hpp"

// void setup() {
//     Serial.begin(115200);
    
//     // Initialize WiFi
//     if (!WiFiManager::begin()) {
//         Serial.println("Failed to initialize WiFi!");
//         return;
//     }
    
//     // Initialize I2S audio
//     if (!I2SAudio::begin()) {
//         Serial.println("Failed to initialize I2S audio!");
//         return;
//     }
// }

// void loop() {
//     Serial.println("\n=== NEW CYCLE ===");
    
//     // Update WiFi status LED
//     WiFiManager::updateStatus();
    
//     // Only proceed if WiFi is connected
//     if (WiFiManager::isConnected()) {
//         if (I2SAudio::recordAndSend()) {
//             delay(500);              // give server time
//             I2SAudio::fetchAndPlay(); // up to 5s worth (server will cut)
//         }
//     }
    
//     delay(2000);  // before next cycle
// } 
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "wifi_manager.hpp"


#define I2S_WS 41
#define I2S_SD 39
#define I2S_SCK 40

// #define SAMPLE_RATE 16000
// #define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define CHANNEL_FORMAT I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_PORT I2S_NUM_0

// #define CHUNK_SIZE 512
#define MAX_AUDIO_BYTES 96000
#define WAV_HEADER_SIZE 44

uint8_t audioBuffer[MAX_AUDIO_BYTES];
uint8_t wavBuffer[WAV_HEADER_SIZE + MAX_AUDIO_BYTES];
size_t currentOffset = 0;
// Replace macros with real constants
const uint32_t SAMPLE_RATE = 16000;
const uint8_t BITS_PER_SAMPLE = 16;
const uint8_t CHANNELS = 1;
const uint32_t MAX_RECORD_TIME_SEC = 3;
const size_t CHUNK_SIZE = 512;


void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = CHANNEL_FORMAT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void recordAudio() {
  currentOffset = 0;
  size_t bytesRead;
  unsigned long startTime = millis();

  Serial.println("Recording 3 seconds...");
  while (millis() - startTime < 3000 && currentOffset + CHUNK_SIZE <= MAX_AUDIO_BYTES) {
    i2s_read(I2S_PORT, &audioBuffer[currentOffset], CHUNK_SIZE, &bytesRead, portMAX_DELAY);
    currentOffset += bytesRead;
  }
  Serial.printf("Recording done. Bytes recorded: %d\n", currentOffset);
}

void addWavHeader(uint8_t* wav, uint32_t pcmDataSize) {
  uint32_t byteRate = SAMPLE_RATE * 2;
  uint32_t chunkSize = 36 + pcmDataSize;

  memcpy(wav + 0, "RIFF", 4);
  memcpy(wav + 8, "WAVE", 4);
  memcpy(wav + 12, "fmt ", 4);

  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1;
  uint16_t numChannels = 1;
  uint16_t bitsPerSample = 16;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;

  memcpy(wav + 4, &chunkSize, 4);
  memcpy(wav + 16, &subchunk1Size, 4);
  memcpy(wav + 20, &audioFormat, 2);
  memcpy(wav + 22, &numChannels, 2);
  memcpy(wav + 24, &SAMPLE_RATE, 4);
  memcpy(wav + 28, &byteRate, 4);
  memcpy(wav + 32, &blockAlign, 2);
  memcpy(wav + 34, &bitsPerSample, 2);

  memcpy(wav + 36, "data", 4);
  memcpy(wav + 40, &pcmDataSize, 4);
}


void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void sendWavToServer(uint8_t* data, size_t len) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "audio/wav");

    int httpResponseCode = http.POST(data, len);

    if (httpResponseCode > 0) {
      Serial.printf("Server response: %d\n", httpResponseCode);
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.printf("Error sending file: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}


void setup() {
  Serial.begin(115200);
  setupI2S();
  delay(1000);
  recordAudio();

  // Add WAV header + copy audio
  addWavHeader(wavBuffer, currentOffset);
  memcpy(wavBuffer + WAV_HEADER_SIZE, audioBuffer, currentOffset);

  Serial.println("WAV file in memory, ready to send or save.");
    // Connect to WiFi and send the WAV file

    if (WiFiManager::begin()) {
        Serial.println("Wi-Fi Connected!");
    } else {
        Serial.println("Failed to connect to Wi-Fi.");
    }
  sendWavToServer(wavBuffer, WAV_HEADER_SIZE + currentOffset);
}
void loop() {
    
  // Placeholder for upload/save
}