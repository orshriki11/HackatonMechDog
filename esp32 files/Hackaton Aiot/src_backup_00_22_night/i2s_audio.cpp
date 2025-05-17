#include "i2s_audio.hpp"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <driver/i2s.h>

uint8_t I2SAudio::audioBuffer[MAX_AUDIO_BYTES];
uint8_t I2SAudio::wavBuffer[WAV_HEADER_SIZE + MAX_AUDIO_BYTES];
size_t I2SAudio::currentOffset = 0;

void I2SAudio::configureI2S() {
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = CHANNEL_FORMAT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_PORT, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_MONO);
}

bool I2SAudio::begin() {
    configureI2S();
    return true;
}

void I2SAudio::read(int32_t* samples, size_t count) {
    size_t bytes_read = 0;
    i2s_read(I2S_PORT, samples, count * sizeof(int32_t), &bytes_read, portMAX_DELAY);
}

void I2SAudio::stop() {
    i2s_driver_uninstall(I2S_PORT);
}

void I2SAudio::recordAudio() {
    currentOffset = 0;
    size_t bytesRead;
    unsigned long startTime = millis();

    Serial.println("Recording 3 seconds...");
    while (millis() - startTime < (MAX_RECORD_TIME_SEC * 1000) && currentOffset + CHUNK_SIZE <= MAX_AUDIO_BYTES) {
        i2s_read(I2S_PORT, &audioBuffer[currentOffset], CHUNK_SIZE, &bytesRead, portMAX_DELAY);
        currentOffset += bytesRead;
    }
    Serial.printf("Recording done. Bytes recorded: %d\n", currentOffset);
}

void I2SAudio::addWavHeader(uint8_t* wav, uint32_t pcmDataSize) {
    uint32_t byteRate = SAMPLE_RATE * 4;  // 32-bit samples
    uint32_t chunkSize = 36 + pcmDataSize;
    uint32_t sampleRate = SAMPLE_RATE;

    memcpy(wav + 0, "RIFF", 4);
    memcpy(wav + 8, "WAVE", 4);
    memcpy(wav + 12, "fmt ", 4);

    uint32_t subchunk1Size = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = CHANNELS;
    uint16_t bitsPerSample = 32;  // 32-bit samples
    uint16_t blockAlign = numChannels * bitsPerSample / 8;

    memcpy(wav + 4, &chunkSize, 4);
    memcpy(wav + 16, &subchunk1Size, 4);
    memcpy(wav + 20, &audioFormat, 2);
    memcpy(wav + 22, &numChannels, 2);
    memcpy(wav + 24, &sampleRate, 4);
    memcpy(wav + 28, &byteRate, 4);
    memcpy(wav + 32, &blockAlign, 2);
    memcpy(wav + 34, &bitsPerSample, 2);

    memcpy(wav + 36, "data", 4);
    memcpy(wav + 40, &pcmDataSize, 4);
}

void I2SAudio::setupMic() {
    i2s_config_t micCfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = CHANNEL_FORMAT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = CHUNK_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t micPins = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    i2s_driver_install(I2S_MIC_NUM, &micCfg, 0, NULL);
    i2s_set_pin(I2S_MIC_NUM, &micPins);
    i2s_zero_dma_buffer(I2S_MIC_NUM);
}

void I2SAudio::setupSpeaker() {
    i2s_config_t spkCfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = CHANNEL_FORMAT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = CHUNK_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t spkPins = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_SD_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    i2s_driver_install(I2S_SPK_NUM, &spkCfg, 0, NULL);
    i2s_set_pin(I2S_SPK_NUM, &spkPins);
    i2s_zero_dma_buffer(I2S_SPK_NUM);
}

bool I2SAudio::recordAndSend() {
    size_t samples = SAMPLE_RATE * RECORD_SEC * CHANNELS;
    size_t bytesToRead = samples * sizeof(int32_t);

    // Allocate on heap
    int32_t *buf = (int32_t*) malloc(bytesToRead);
    if (!buf) {
        Serial.println("❌ malloc failed");
        return false;
    }

    Serial.println("🎙️ Recording...");
    size_t total=0, r;
    uint8_t *p = (uint8_t*)buf;
    while (total < bytesToRead) {
        i2s_read(I2S_MIC_NUM, p+total, bytesToRead-total, &r, portMAX_DELAY);
        total += r;
    }
    Serial.println("✅ Recorded");

    // HTTP POST
    Serial.println("📡 Uploading...");
    HTTPClient http;
    http.begin(UPLOAD_URL);
    http.addHeader("Content-Type","application/octet-stream");
    int code = http.POST((uint8_t*)buf, bytesToRead);
    http.end();
    free(buf);

    if (code == 200) {
        Serial.println("✅ Server OK");
        return true;
    } else {
        Serial.printf("❌ Upload failed %d\n", code);
        return false;
    }
}

void I2SAudio::fetchAndPlay() {
    Serial.println("⬇️ Fetching /sound…");
    HTTPClient http;
    http.begin(SOUND_URL);
    int code = http.GET();
    if (code != 200) {
        Serial.printf("❌ GET failed %d\n",code);
        http.end();
        return;
    }

    WiFiClient *stream = http.getStreamPtr();
    size_t total=0, r;
    static uint8_t chunk[CHUNK_SIZE];

    Serial.println("🔊 Playing...");
    while (stream->available()) {
        r = stream->read(chunk, CHUNK_SIZE);
        if (r == 0) break;
        size_t written=0;
        i2s_write(I2S_SPK_NUM, chunk, r, &written, portMAX_DELAY);
        total += written;
    }
    http.end();
    Serial.printf("✅ Played %u bytes\n", total);
} 