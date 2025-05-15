// // #include <Arduino.h>
// // #include "driver/i2s.h"

// // #define I2S_WS      35   // Shared LRCL
// // #define I2S_SCK     14   // Shared BCLK
// // #define I2S_SD1     38   // Mic 1 SD
// // #define I2S_SD2     41   // Mic 2 SD

// // #define SAMPLE_SIZE 512

// // int32_t mic1_samples[SAMPLE_SIZE];
// // int32_t mic2_samples[SAMPLE_SIZE];

// // void setupI2S(i2s_port_t port, int dataPin) {
// //   const i2s_config_t i2s_config = {
// //     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = 16000,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
// //     .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
// //     .communication_format = I2S_COMM_FORMAT_I2S,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = 256,
// //     .use_apll = false,
// //     .tx_desc_auto_clear = false,
// //     .fixed_mclk = 0
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = I2S_PIN_NO_CHANGE,
// //     .data_in_num = dataPin
// //   };

// //   i2s_driver_install(port, &i2s_config, 0, NULL);
// //   i2s_set_pin(port, &pin_config);
// //   i2s_zero_dma_buffer(port);
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   delay(1000);
// //   Serial.println("🎤 Initializing two separate I2S microphones...");

// //   setupI2S(I2S_NUM_0, I2S_SD1);
// //   setupI2S(I2S_NUM_1, I2S_SD2);

// //   Serial.println("✅ Both I2S interfaces ready. Listening for sound...");
// // }

// // void loop() {
// //   size_t bytesRead1 = 0, bytesRead2 = 0;
// //   i2s_read(I2S_NUM_0, (void *)mic1_samples, sizeof(mic1_samples), &bytesRead1, portMAX_DELAY);
// //   i2s_read(I2S_NUM_1, (void *)mic2_samples, sizeof(mic2_samples), &bytesRead2, portMAX_DELAY);

// //   int count1 = bytesRead1 / 4;
// //   int count2 = bytesRead2 / 4;

// //   int64_t sum1 = 0, sum2 = 0;

// //   for (int i = 0; i < min(count1, SAMPLE_SIZE); i++) {
// //     sum1 += abs(mic1_samples[i]);
// //   }

// //   for (int i = 0; i < min(count2, SAMPLE_SIZE); i++) {
// //     sum2 += abs(mic2_samples[i]);
// //   }

// //   int avg1 = sum1 / count1;
// //   int avg2 = sum2 / count2;

// //   Serial.print("🎙️ Mic1: ");
// //   Serial.print(avg1);
// //   Serial.print(" | 🎙️ Mic2: ");
// //   Serial.print(avg2);
// //   Serial.printf("Mic1 avg: %llu, Mic2 avg: %llu\n",avg1, avg2);


// //   if (avg1 > avg2 * 1.3) {
// //     Serial.println(" => 🔈 Sound from MIC 1 (GPIO 38)");
// //   } else if (avg2 > avg1 * 1.3) {
// //     Serial.println(" => 🔊 Sound from MIC 2 (GPIO 41)");
// //   } else {
// //     Serial.println(" => 🎧 Sound CENTERED");
// //   }

// //   delay(500);
// // }


// // #include <Arduino.h>
// // #include "driver/i2s.h"
// // #include <math.h>

// // #define I2S_WS      40
// // #define I2S_SD      38
// // #define I2S_SCK     39

// // #define LED_LEFT    1
// // #define LED_RIGHT   47

// // #define SAMPLE_RATE     16000
// // #define BUFFER_LEN      1024  // Must be power of 2
// // #define CAPTURE_TIME_MS 250

// // float mic_left[BUFFER_LEN];
// // float mic_right[BUFFER_LEN];
// // int16_t i2s_read_buff[BUFFER_LEN * 2];

// // void setupI2SMics() {
// //   const i2s_config_t i2s_config = {
// //     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = SAMPLE_RATE,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
// //     .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Read only one slot, decode manually
// //     .communication_format = I2S_COMM_FORMAT_I2S,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = BUFFER_LEN,
// //     .use_apll = false,
// //     .tx_desc_auto_clear = false,
// //     .fixed_mclk = 0
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = I2S_PIN_NO_CHANGE,
// //     .data_in_num = I2S_SD
// //   };

// //   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
// //   i2s_set_pin(I2S_NUM_0, &pin_config);
// // }

// // void setupLEDs() {
// //   pinMode(LED_LEFT, OUTPUT);
// //   pinMode(LED_RIGHT, OUTPUT);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// // }

// // void getSamples() {
// //   size_t bytes_read;
// //   i2s_read(I2S_NUM_0, &i2s_read_buff, sizeof(i2s_read_buff), &bytes_read, portMAX_DELAY);

// //   int samples_read = bytes_read / sizeof(int32_t);
// //   for (int i = 0; i < samples_read; i++) {
// //     int32_t sample = i2s_read_buff[i];
// //     int32_t channel = i % 2;

// //     float normalized = sample / 2147483648.0f;
// //     if (channel == 0) {
// //       mic_left[i / 2] = normalized;
// //     } else {
// //       mic_right[i / 2] = normalized;
// //     }
// //   }
// // }

// // // GCC-PHAT estimation
// // int estimateDelay(float *sig1, float *sig2, int len) {
// //   float max_corr = -1e9;
// //   int best_lag = 0;
// //   const int max_lag = 20;  // Tune this for 5cm mics @ 16kHz

// //   for (int lag = -max_lag; lag <= max_lag; lag++) {
// //     float sum = 0;
// //     for (int i = 0; i < len; i++) {
// //       int j = i + lag;
// //       if (j >= 0 && j < len) {
// //         sum += sig1[i] * sig2[j];
// //       }
// //     }

// //     if (sum > max_corr) {
// //       max_corr = sum;
// //       best_lag = lag;
// //     }
// //   }

// //   return best_lag;
// // }

// // void indicateDirection(int delay) {
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);

// //   if (delay < -2) {
// //     digitalWrite(LED_LEFT, HIGH);  // Left arrives first
// //   } else if (delay > 2) {
// //     digitalWrite(LED_RIGHT, HIGH); // Right arrives first
// //   }
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   setupLEDs();
// //   setupI2SMics();
// //   Serial.println("Sound direction detection started...");
// // }

// // void loop() {
// //   getSamples();
// //   int delaylen = estimateDelay(mic_left, mic_right, BUFFER_LEN / 2);
// //   indicateDirection(delaylen);

// //   Serial.printf("Estimated delay: %d samples\n", delay);
// //   delay(CAPTURE_TIME_MS);
// // }
// // #include <Arduino.h>
// // #include <driver/i2s.h>
// // #include <arduinoFFT.h>

// // #define I2S_WS 39
// // #define I2S_SD 38
// // #define I2S_SCK 40
// // #define LED_LEFT 1
// // #define LED_RIGHT 47

// // #define SAMPLE_RATE     16000
// // #define BUFFER_SIZE     512  // Must be power of two
// // #define LED_THRESHOLD   10   // Samples of TDOA threshold (~0.625ms)

// // int16_t i2sBuffer[BUFFER_SIZE * 2];  // interleaved: [L,R,L,R,...]
// // float micLeft[BUFFER_SIZE];
// // float micRight[BUFFER_SIZE];

// // // I2S config
// // void setupI2S() {
// //   const i2s_config_t i2s_config = {
// //     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = SAMPLE_RATE,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
// //     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
// //     .communication_format = I2S_COMM_FORMAT_STAND_MSB,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = BUFFER_SIZE,
// //     .use_apll = false
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = -1,
// //     .data_in_num = I2S_SD
// //   };

// //   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
// //   i2s_set_pin(I2S_NUM_0, &pin_config);
// //   i2s_zero_dma_buffer(I2S_NUM_0);
// // }

// // // GCC-PHAT
// // int estimateDelay(float *x, float *y, int n) {
// //   arduinoFFT FFT = arduinoFFT();

// //   double Xre[n], Xim[n], Yre[n], Yim[n], Rre[n], Rim[n];

// //   for (int i = 0; i < n; i++) {
// //     Xre[i] = x[i];
// //     Xim[i] = 0;
// //     Yre[i] = y[i];
// //     Yim[i] = 0;
// //   }

// //   FFT.Compute(Xre, Xim, n, FFT_FORWARD);
// //   FFT.Compute(Yre, Yim, n, FFT_FORWARD);

// //   for (int i = 0; i < n; i++) {
// //     double denom = sqrt(pow(Xre[i], 2) + pow(Xim[i], 2)) * sqrt(pow(Yre[i], 2) + pow(Yim[i], 2)) + 1e-9;
// //     Rre[i] = (Xre[i] * Yre[i] + Xim[i] * Yim[i]) / denom;
// //     Rim[i] = (Xim[i] * Yre[i] - Xre[i] * Yim[i]) / denom;
// //   }

// //   FFT.Compute(Rre, Rim, n, FFT_BACKWARD);

// //   int maxIndex = 0;
// //   double maxVal = -1e9;
// //   for (int i = 0; i < n; i++) {
// //     double val = Rre[i];
// //     if (val > maxVal) {
// //       maxVal = val;
// //       maxIndex = i;
// //     }
// //   }

// //   int delay = maxIndex;
// //   if (delay > n / 2) delay -= n; // wrap around

// //   return delay;
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   pinMode(LED_LEFT, OUTPUT);
// //   pinMode(LED_RIGHT, OUTPUT);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   setupI2S();
// // }

// // void loop() {
// //   size_t bytesRead;
// //   i2s_read(I2S_NUM_0, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

// //   int samples = bytesRead / sizeof(int16_t) / 2;  // 2 channels
// //   for (int i = 0; i < samples; i++) {
// //     micLeft[i]  = (float)i2sBuffer[i * 2];     // L
// //     micRight[i] = (float)i2sBuffer[i * 2 + 1]; // R
// //   }

// //   int delaytime = estimateDelay(micLeft, micRight, samples);
// //   Serial.print("Estimated delay: ");
// //   Serial.print(delaytime);
// //   Serial.print(" samples\t");

// //   if (delaytime > LED_THRESHOLD) {
// //     Serial.println("→ Sound from LEFT");
// //     digitalWrite(LED_LEFT, HIGH);
// //     digitalWrite(LED_RIGHT, LOW);
// //   } else if (delaytime < -LED_THRESHOLD) {
// //     Serial.println("→ Sound from RIGHT");
// //     digitalWrite(LED_LEFT, LOW);
// //     digitalWrite(LED_RIGHT, HIGH);
// //   } else {
// //     Serial.println("→ Sound from CENTER");
// //     digitalWrite(LED_LEFT, LOW);
// //     digitalWrite(LED_RIGHT, LOW);
// //   }

// //   delay(250);  // 4 times per second
// // // }
// // #include <Arduino.h>
// // #include <driver/i2s.h>

// // #define I2S_WS 39
// // #define I2S_SD 38
// // #define I2S_SCK 40
// // #define LED_LEFT 1
// // #define LED_RIGHT 47

// // #define SAMPLE_RATE     16000
// // #define BUFFER_SIZE     512  // Must be power of two
// // #define LED_THRESHOLD   10   // Delay threshold in samples

// // int16_t i2sBuffer[BUFFER_SIZE * 2];  // interleaved: [L,R,L,R,...]
// // float micLeft[BUFFER_SIZE];
// // float micRight[BUFFER_SIZE];

// // // I2S setup
// // void setupI2S() {
// //   const i2s_config_t i2s_config = {
// //     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = SAMPLE_RATE,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
// //     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
// //     .communication_format = I2S_COMM_FORMAT_STAND_MSB,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = BUFFER_SIZE,
// //     .use_apll = false
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = -1,
// //     .data_in_num = I2S_SD
// //   };

// //   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
// //   i2s_set_pin(I2S_NUM_0, &pin_config);
// //   i2s_zero_dma_buffer(I2S_NUM_0);
// // }

// // // Time domain cross-correlation, lag range limited to ±maxLag
// // int estimateDelay(float* x, float* y, int n, int maxLag) {
// //   int bestLag = 0;
// //   float maxCorr = -1e9;

// //   for (int lag = -maxLag; lag <= maxLag; lag++) {
// //     float corr = 0.0f;

// //     for (int i = 0; i < n; i++) {
// //       int j = i + lag;
// //       if (j < 0 || j >= n) continue;
// //       corr += x[i] * y[j];
// //     }

// //     if (corr > maxCorr) {
// //       maxCorr = corr;
// //       bestLag = lag;
// //     }
// //   }

// //   return bestLag;
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   pinMode(LED_LEFT, OUTPUT);
// //   pinMode(LED_RIGHT, OUTPUT);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   setupI2S();
// // }

// // void loop() {
// //   size_t bytesRead;
// //   i2s_read(I2S_NUM_0, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

// //   int samples = bytesRead / sizeof(int16_t) / 2;  // 2 channels
// //   for (int i = 0; i < samples; i++) {
// //     micLeft[i]  = (float)i2sBuffer[i * 2];     // Left channel
// //     micRight[i] = (float)i2sBuffer[i * 2 + 1]; // Right channel
// //   }

// //   // Estimate delay with max lag = LED_THRESHOLD * 2 (20 samples)
// //   int delaytime = estimateDelay(micLeft, micRight, samples, LED_THRESHOLD * 2);

// //   Serial.print("Estimated delay: ");
// //   Serial.print(delaytime);
// //   Serial.print(" samples\t");

// //   if (delaytime > LED_THRESHOLD) {
// //     Serial.println("→ Sound from LEFT");
// //     digitalWrite(LED_LEFT, HIGH);
// //     digitalWrite(LED_RIGHT, LOW);
// //   } else if (delaytime < -LED_THRESHOLD) {
// //     Serial.println("→ Sound from RIGHT");
// //     digitalWrite(LED_LEFT, LOW);
// //     digitalWrite(LED_RIGHT, HIGH);
// //   } else {
// //     Serial.println("→ Sound from CENTER");
// //     digitalWrite(LED_LEFT, LOW);
// //     digitalWrite(LED_RIGHT, LOW);
// //   }

// //   delay(250);  // 4 Hz update rate
// // }

// // #include <Arduino.h>
// // #include <driver/i2s.h>

// // #define I2S_WS 39
// // #define I2S_SD 38
// // #define I2S_SCK 40
// // #define LED_LEFT 1
// // #define LED_RIGHT 47

// // #define SAMPLE_RATE     16000
// // #define BUFFER_SIZE     512
// // #define AMPLITUDE_THRESHOLD 1000  // Adjust based on your mic signal strength

// // int16_t i2sBuffer[BUFFER_SIZE * 2];  // interleaved: [L,R,L,R,...]
// // float micLeft[BUFFER_SIZE];
// // float micRight[BUFFER_SIZE];

// // void setupI2S() {
// //   const i2s_config_t i2s_config = {
// //     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = SAMPLE_RATE,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
// //     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
// //     .communication_format = I2S_COMM_FORMAT_STAND_MSB,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = BUFFER_SIZE,
// //     .use_apll = false
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = -1,
// //     .data_in_num = I2S_SD
// //   };

// //   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
// //   i2s_set_pin(I2S_NUM_0, &pin_config);
// //   i2s_zero_dma_buffer(I2S_NUM_0);
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   pinMode(LED_LEFT, OUTPUT);
// //   pinMode(LED_RIGHT, OUTPUT);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   setupI2S();
// // }

// // void loop() {
// //   size_t bytesRead;
// //   i2s_read(I2S_NUM_0, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

// //   int samples = bytesRead / sizeof(int16_t) / 2;  // 2 channels

// //   long sumLeft = 0;
// //   long sumRight = 0;

// //   for (int i = 0; i < samples; i++) {
// //     int16_t leftSample = i2sBuffer[i * 2];
// //     int16_t rightSample = i2sBuffer[i * 2 + 1];

// //     micLeft[i] = leftSample;
// //     micRight[i] = rightSample;

// //     // Use absolute value to get amplitude
// //     sumLeft += abs(leftSample);
// //     sumRight += abs(rightSample);
// //   }

// //   // Calculate average amplitude for left and right
// //   float avgLeft = (float)sumLeft / samples;
// //   float avgRight = (float)sumRight / samples;

// //   Serial.print("Left avg: ");
// //   Serial.print(avgLeft);
// //   Serial.print("\tRight avg: ");
// //   Serial.println(avgRight);

// //   if (avgLeft - avgRight > AMPLITUDE_THRESHOLD) {
// //     digitalWrite(LED_LEFT, HIGH);
// //     digitalWrite(LED_RIGHT, LOW);
// //   } else if (avgRight - avgLeft > AMPLITUDE_THRESHOLD) {
// //     digitalWrite(LED_LEFT, LOW);
// //     digitalWrite(LED_RIGHT, HIGH);
// //   } else {
// //     digitalWrite(LED_LEFT, HIGH);
// //     digitalWrite(LED_RIGHT, HIGH);
// //   }
// //   delay(100);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   // Small delay to avoid flickering
// //   delay(250);
// // }

// // #include <Arduino.h>
// // #include <driver/i2s.h>

// // #define I2S_WS 35
// // #define I2S_SD 38
// // #define I2S_SCK 14
// // #define LED_LEFT 1
// // #define LED_RIGHT 47

// // #define SAMPLE_RATE     16000
// // #define BUFFER_SIZE     512
// // #define AMPLITUDE_THRESHOLD 1000  // Adjust based on your mic signal strength

// // int16_t i2sBuffer[BUFFER_SIZE * 2];  // interleaved: [L,R,L,R,...]
// // float micLeft[BUFFER_SIZE];
// // float micRight[BUFFER_SIZE];

// // void setupI2S() {
// //   const i2s_config_t i2s_config = {
// //     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = SAMPLE_RATE,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
// //     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
// //     .communication_format = I2S_COMM_FORMAT_STAND_MSB,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = BUFFER_SIZE,
// //     .use_apll = false
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = -1,
// //     .data_in_num = I2S_SD
// //   };

// //   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
// //   i2s_set_pin(I2S_NUM_0, &pin_config);
// //   i2s_zero_dma_buffer(I2S_NUM_0);
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   pinMode(LED_LEFT, OUTPUT);
// //   pinMode(LED_RIGHT, OUTPUT);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   setupI2S();
// // }

// // void loop() {
// //   size_t bytesRead;
// //   i2s_read(I2S_NUM_0, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

// //   int samples = bytesRead / sizeof(int16_t) / 2;  // 2 channels

// //   // --- Debug print first 10 samples ---
// //   Serial.println("First 10 samples (L R):");
// //   for (int i = 0; i < 10 && i < samples; i++) {
// //     Serial.print(i2sBuffer[i * 2]);
// //     Serial.print(" ");
// //     Serial.println(i2sBuffer[i * 2 + 1]);
// //   }

// //   long sumLeft = 0;
// //   long sumRight = 0;
// //   int16_t maxLeft = 0;
// //   int16_t maxRight = 0;

// //   for (int i = 0; i < samples; i++) {
// //     int16_t leftSample = i2sBuffer[i * 2];
// //     int16_t rightSample = i2sBuffer[i * 2 + 1];

// //     micLeft[i] = leftSample;
// //     micRight[i] = rightSample;

// //     int16_t absLeft = abs(leftSample);
// //     int16_t absRight = abs(rightSample);

// //     sumLeft += absLeft;
// //     sumRight += absRight;

// //     if (absLeft > maxLeft) maxLeft = absLeft;
// //     if (absRight > maxRight) maxRight = absRight;
// //   }

// //   float avgLeft = (float)sumLeft / samples;
// //   float avgRight = (float)sumRight / samples;

// //   Serial.print("Left avg: ");
// //   Serial.print(avgLeft);
// //   Serial.print("\tRight avg: ");
// //   Serial.print(avgRight);
// //   Serial.print("\tMax Left: ");
// //   Serial.print(maxLeft);
// //   Serial.print("\tMax Right: ");
// //   Serial.println(maxRight);

// //   if (avgLeft - avgRight > AMPLITUDE_THRESHOLD) {
// //     digitalWrite(LED_LEFT, HIGH);
// //     digitalWrite(LED_RIGHT, LOW);
// //   } else if (avgRight - avgLeft > AMPLITUDE_THRESHOLD) {
// //     digitalWrite(LED_LEFT, LOW);
// //     digitalWrite(LED_RIGHT, HIGH);
// //   } else {
// //     digitalWrite(LED_LEFT, HIGH);
// //     digitalWrite(LED_RIGHT, HIGH);
// //   }

// //   delay(100);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   delay(250);
// // }


// // #include <Arduino.h>
// // #include <driver/i2s.h>

// // // I2S Pin Definitions
// // #define I2S_WS 35    // LRCK / WS
// // #define I2S_SD 38    // DOUT from mics
// // #define I2S_SCK 14   // BCLK

// // // LED Pins
// // #define LED_LEFT 1
// // #define LED_RIGHT 47

// // // Constants
// // #define SAMPLE_RATE     16000
// // #define BUFFER_SIZE     512
// // #define AMPLITUDE_THRESHOLD 1000
// // #define LOUD_SOUND_THRESHOLD 10000  // Adjust for loud voice/scream

// // // Buffers
// // int16_t i2sBuffer[BUFFER_SIZE * 2];  // [L,R,L,R,...]
// // float micLeft[BUFFER_SIZE];
// // float micRight[BUFFER_SIZE];

// // void setupI2S() {
// //   const i2s_config_t i2s_config = {
// //     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
// //     .sample_rate = SAMPLE_RATE,
// //     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
// //     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
// //     .communication_format = I2S_COMM_FORMAT_STAND_MSB,
// //     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
// //     .dma_buf_count = 4,
// //     .dma_buf_len = BUFFER_SIZE,
// //     .use_apll = false
// //   };

// //   const i2s_pin_config_t pin_config = {
// //     .bck_io_num = I2S_SCK,
// //     .ws_io_num = I2S_WS,
// //     .data_out_num = -1,
// //     .data_in_num = I2S_SD
// //   };

// //   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
// //   i2s_set_pin(I2S_NUM_0, &pin_config);
// //   i2s_zero_dma_buffer(I2S_NUM_0);
// // }

// // void setup() {
// //   Serial.begin(115200);
// //   pinMode(LED_LEFT, OUTPUT);
// //   pinMode(LED_RIGHT, OUTPUT);
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);
// //   setupI2S();
// // }

// // void loop() {
// //   size_t bytesRead;
// //   i2s_read(I2S_NUM_0, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

// //   int samples = bytesRead / sizeof(int16_t) / 2;  // Stereo
// //   long sumLeft = 0, sumRight = 0;
// //   int16_t maxLeft = 0, maxRight = 0;

// //   for (int i = 0; i < samples; i++) {
// //     int16_t leftSample = i2sBuffer[i * 2];
// //     int16_t rightSample = i2sBuffer[i * 2 + 1];

// //     micLeft[i] = leftSample;
// //     micRight[i] = rightSample;

// //     sumLeft += abs(leftSample);
// //     sumRight += abs(rightSample);

// //     if (abs(leftSample) > maxLeft) maxLeft = abs(leftSample);
// //     if (abs(rightSample) > maxRight) maxRight = abs(rightSample);
// //   }

// //   float avgLeft = (float)sumLeft / samples;
// //   float avgRight = (float)sumRight / samples;

// //   // Always show ongoing values
// //   Serial.print("Left avg: "); Serial.print(avgLeft);
// //   Serial.print("  Right avg: "); Serial.print(avgRight);
// //   Serial.print("  Max Left: "); Serial.print(maxLeft);
// //   Serial.print("  Max Right: "); Serial.println(maxRight);

// //   // Reset LEDs
// //   digitalWrite(LED_LEFT, LOW);
// //   digitalWrite(LED_RIGHT, LOW);

// //   // Loud sound detection
// //   if (avgLeft > LOUD_SOUND_THRESHOLD || avgRight > LOUD_SOUND_THRESHOLD) {
// //     if (avgLeft - avgRight > AMPLITUDE_THRESHOLD) {
// //       digitalWrite(LED_LEFT, HIGH);
// //       Serial.println("🔊 Loud sound detected from LEFT!");
// //     } else if (avgRight - avgLeft > AMPLITUDE_THRESHOLD) {
// //       digitalWrite(LED_RIGHT, HIGH);
// //       Serial.println("🔊 Loud sound detected from RIGHT!");
// //     } else {
// //       digitalWrite(LED_LEFT, HIGH);
// //       digitalWrite(LED_RIGHT, HIGH);
// //       Serial.println("🔊 Loud sound detected from CENTER!");
// //     }
// //   }

// //   delay(100);  // Adjust if needed
// // }


// #include <Arduino.h>
// #include <driver/i2s.h>
// #include <math.h>

// // I2S Pin Definitions
// #define I2S_WS 35    // LRCK / WS
// #define I2S_SD 38    // DOUT from mics
// #define I2S_SCK 14   // BCLK

// // LED Pins
// #define LED_LEFT 1
// #define LED_RIGHT 47

// // Constants
// #define SAMPLE_RATE     16000
// #define BUFFER_SIZE     512
// #define AMPLITUDE_THRESHOLD 1000
// #define LOUD_SOUND_THRESHOLD 1000000  // Adjust for loud voice/scream

// // Buffers
// int32_t i2sBuffer[BUFFER_SIZE * 2];  // [L,R,L,R,...] as 32-bit packed
// float micLeft[BUFFER_SIZE];
// float micRight[BUFFER_SIZE];

// // Delay Estimation Function (basic GCC-PHAT style)
// int estimateDelay(float *sig1, float *sig2, int len) {
//   float max_corr = -1e9;
//   int best_lag = 0;
//   const int max_lag = 20;  // Roughly ±1.25ms @ 16kHz (21.25 cm at speed of sound)

//   for (int lag = -max_lag; lag <= max_lag; lag++) {
//     float sum = 0;
//     for (int i = 0; i < len; i++) {
//       int j = i + lag;
//       if (j >= 0 && j < len) {
//         sum += sig1[i] * sig2[j];
//       }
//     }

//     if (sum > max_corr) {
//       max_corr = sum;
//       best_lag = lag;
//     }
//   }

//   return best_lag;
// }

// void setupI2S() {
//   const i2s_config_t i2s_config = {
//     .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
//     .sample_rate = SAMPLE_RATE,
//     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
//     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//     .communication_format = I2S_COMM_FORMAT_STAND_MSB,
//     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//     .dma_buf_count = 4,
//     .dma_buf_len = BUFFER_SIZE,
//     .use_apll = false
//   };

//   const i2s_pin_config_t pin_config = {
//     .bck_io_num = I2S_SCK,
//     .ws_io_num = I2S_WS,
//     .data_out_num = -1,
//     .data_in_num = I2S_SD
//   };

//   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//   i2s_set_pin(I2S_NUM_0, &pin_config);
//   i2s_zero_dma_buffer(I2S_NUM_0);
// }

// void setup() {
//   Serial.begin(115200);
//   pinMode(LED_LEFT, OUTPUT);
//   pinMode(LED_RIGHT, OUTPUT);
//   digitalWrite(LED_LEFT, LOW);
//   digitalWrite(LED_RIGHT, LOW);
//   setupI2S();
// }

// void loop() {
//   size_t bytesRead;
//   i2s_read(I2S_NUM_0, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

//   int samples = bytesRead / sizeof(int32_t) / 2;  // Stereo samples
//   long sumLeft = 0, sumRight = 0;
//   int16_t maxLeft = 0, maxRight = 0;

//   for (int i = 0; i < samples; i++) {
//     // Extract 24-bit mic data from 32-bit values (left/right)
//     int32_t leftRaw = i2sBuffer[i * 2] >> 8;   // discard lowest 8 bits
//     int32_t rightRaw = i2sBuffer[i * 2 + 1] >> 8;

//     micLeft[i] = leftRaw / 8388608.0f;   // Normalize 24-bit signed
//     micRight[i] = rightRaw / 8388608.0f;

//     sumLeft += abs(leftRaw);
//     sumRight += abs(rightRaw);

//     if (abs(leftRaw) > maxLeft) maxLeft = abs(leftRaw);
//     if (abs(rightRaw) > maxRight) maxRight = abs(rightRaw);
//   }

//   float avgLeft = (float)sumLeft / samples;
//   float avgRight = (float)sumRight / samples;

//   // Always show ongoing values
//   Serial.print("Left avg: "); Serial.print(avgLeft);
//   Serial.print("  Right avg: "); Serial.print(avgRight);
//   Serial.print("  Max Left: "); Serial.print(maxLeft);
//   Serial.print("  Max Right: "); Serial.print(maxRight);

//   // Estimate delay
//   int bestLag = estimateDelay(micLeft, micRight, samples);
//   Serial.print("  Lag: "); Serial.println(bestLag);

//   digitalWrite(LED_LEFT, LOW);
//   digitalWrite(LED_RIGHT, LOW);

//   // Loud sound detection based on amplitude
//   if (avgLeft > LOUD_SOUND_THRESHOLD || avgRight > LOUD_SOUND_THRESHOLD) {
//     if (bestLag < -3) {
//       digitalWrite(LED_LEFT, HIGH);
//       Serial.println("🔊 Loud sound from LEFT!");
//     } else if (bestLag > 3) {
//       digitalWrite(LED_RIGHT, HIGH);
//       Serial.println("🔊 Loud sound from RIGHT!");
//     } else {
//       digitalWrite(LED_LEFT, HIGH);
//       digitalWrite(LED_RIGHT, HIGH);
//       Serial.println("🔊 Loud sound from CENTER!");
//     }
//   }
//   delay(100);
//         digitalWrite(LED_LEFT, LOW);
//       digitalWrite(LED_RIGHT, LOW);
      
//   delay(250);  // Run every 0.25s
// }