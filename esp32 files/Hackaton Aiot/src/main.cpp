#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>
#include "led_functions.hpp"

// WiFi Credentials
const char* ssid = "Mechdog";
const char* password = "Mechdog1!";

// Server URL
const char* serverUrl = "http://192.168.137.1:12345/upload";  // Replace with your PC's IP

// I2S Pins (adjust if needed)
#define I2S_WS 35
#define I2S_SD 38
#define I2S_SCK 14

// Audio Config
#define SAMPLE_RATE 16000
#define RECORD_SECONDS 1
#define SAMPLE_COUNT (SAMPLE_RATE * RECORD_SECONDS)
#define I2S_NUM I2S_NUM_0

int16_t audioBuffer[SAMPLE_COUNT];

void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  turnOnBlueLED(); // 🔵 On when connected
  
}

void setupI2S() {
  i2s_config_t config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 512,
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

  i2s_driver_install(I2S_NUM, &config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM);
}

void recordAudio() {
  size_t bytesRead = 0;
  int16_t* ptr = audioBuffer;
  size_t totalRead = 0;

  Serial.println("🎙️ Recording...");

  while (totalRead < SAMPLE_COUNT * sizeof(int16_t)) {
    i2s_read(I2S_NUM, ptr, (SAMPLE_COUNT * sizeof(int16_t)) - totalRead, &bytesRead, portMAX_DELAY);
    ptr += bytesRead / sizeof(int16_t);
    totalRead += bytesRead;
  }

  Serial.println("✅ Recording done");
}

// Send a simple text message to server instead of audio
void sendToServer() {
  Serial.println("📡 Sending test message...");

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "text/plain");

    int httpResponseCode = http.POST("Hello from ESP32-S3!");

    if (httpResponseCode > 0) {
      Serial.print("✅ Server responded: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println("📬 Response: " + payload);
      blinkRedLED();  // 🔴 blink on successful response
    } else {
      Serial.print("❌ Error sending: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("❌ WiFi disconnected");
  }
}
// Uncomment the following function to send audio data instead of a text message
// void sendToServer() {
//   Serial.println("📡 Sending audio...");

//   if ((WiFi.status() == WL_CONNECTED)) {
//     HTTPClient http;
//     http.begin(serverUrl);
//     http.addHeader("Content-Type", "application/octet-stream");

//     int httpResponseCode = http.POST((uint8_t*)audioBuffer, sizeof(audioBuffer));

//     if (httpResponseCode > 0) {
//       Serial.print("✅ Server responded: ");
//       Serial.println(httpResponseCode);
//       String payload = http.getString();
//       Serial.println("📬 Response: " + payload);
//     } else {
//       Serial.print("❌ Error sending: ");
//       Serial.println(http.errorToString(httpResponseCode));
//     }

//     http.end();
//   } else {
//     Serial.println("❌ WiFi disconnected");
//   }
// }

void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupI2S();
  setupLEDs();
  
}

void loop() {
  delay(3000); // Wait before each sample
  recordAudio();
  sendToServer();
}