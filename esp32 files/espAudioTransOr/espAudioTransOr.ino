#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <algorithm>
#include "driver/i2s.h"

#define I2S_WS      41
#define I2S_SCK     40
#define I2S_SD      39

#define SAMPLE_RATE     16000
#define SAMPLE_BITS     I2S_BITS_PER_SAMPLE_16BIT
#define I2S_PORT        I2S_NUM_0
#define CHANNEL_COUNT   1
#define RECORD_SECONDS  5
#define BYTES_PER_SAMPLE 2  // 16 bits = 2 bytes

#define RECORD_SIZE     (SAMPLE_RATE * RECORD_SECONDS * BYTES_PER_SAMPLE) // 160000 bytes
#define CHUNK_SIZE      512
#define SERVER_URL      "http://192.168.137.1:12345/upload"

const char* ssid = "Or-MechDog2";
const char* password = "or123456";

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());
}

void setupI2S() {
  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = SAMPLE_BITS,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = CHUNK_SIZE / 2,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &config, 0, nullptr);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void sendBuffer(uint8_t* buffer, size_t len) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Skipping send.");
    return;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/octet-stream");

  int httpResponseCode = http.POST(buffer, len);
  Serial.printf("POST %d bytes -> HTTP %d\n", len, httpResponseCode);

  http.end();
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupI2S();
}
void loop() {
  static uint8_t audioBuffer[RECORD_SIZE];
  size_t totalRead = 0;

  Serial.println("Recording 5 seconds of audio...");

  while (totalRead < RECORD_SIZE) {
    size_t bytesToRead = (CHUNK_SIZE < (RECORD_SIZE - totalRead)) ? CHUNK_SIZE : (RECORD_SIZE - totalRead);
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, audioBuffer + totalRead, bytesToRead, &bytesRead, portMAX_DELAY);

    if (result == ESP_OK && bytesRead > 0) {
      totalRead += bytesRead;
    } else {
      Serial.println("I2S read failed.");
      break;
    }
  }
  Serial.printf("Recording complete: %d bytes\nSending to server...\n", totalRead);
  sendBuffer(audioBuffer, totalRead);

  delay(6000); // Wait 1 second more than record time
}