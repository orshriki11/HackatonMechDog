#include "i2s_audio.hpp"
#include <Arduino.h>

void I2SAudio::setupMic() {
    i2s_config_t micCfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t micPins = {
        .bck_io_num = I2S_BCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD_MIC
    };
    
    i2s_driver_install(I2S_MIC_NUM, &micCfg, 0, NULL);
    i2s_set_pin(I2S_MIC_NUM, &micPins);
    i2s_zero_dma_buffer(I2S_MIC_NUM);
}

void I2SAudio::setupSpeaker() {
    i2s_config_t spkCfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER|I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t spkPins = {
        .bck_io_num = I2S_BCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_SD_SPK,
        .data_in_num = -1
    };
    
    i2s_driver_install(I2S_SPK_NUM, &spkCfg, 0, NULL);
    i2s_set_pin(I2S_SPK_NUM, &spkPins);
    i2s_zero_dma_buffer(I2S_SPK_NUM);
}

bool I2SAudio::begin() {
    setupMic();
    setupSpeaker();
    return true;
}

bool I2SAudio::recordAndSend() {
    size_t samples = SAMPLE_RATE * RECORD_SEC * CHANNELS;
    size_t bytesToRead = samples * sizeof(int16_t);

    // Allocate on heap
    int16_t *buf = (int16_t*) malloc(bytesToRead);
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
    http.begin(uploadURL);
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
    http.begin(soundURL);
    int code = http.GET();
    if (code != 200) {
        Serial.printf("❌ GET failed %d\n",code);
        http.end();
        return;
    }

    WiFiClient *stream = http.getStreamPtr();
    size_t total=0, r;
    static uint8_t chunk[CHUNK_BYTES];

    Serial.println("🔊 Playing...");
    while (stream->available()) {
        r = stream->read(chunk, CHUNK_BYTES);
        if (r == 0) break;
        size_t written=0;
        i2s_write(I2S_SPK_NUM, chunk, r, &written, portMAX_DELAY);
        total += written;
    }
    http.end();
    Serial.printf("✅ Played %u bytes\n", total);
} 