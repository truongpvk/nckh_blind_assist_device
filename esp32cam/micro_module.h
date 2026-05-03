#ifndef MICRO_MODULE_H
#define MICRO_MODULE_H

// ============================================================
// micro_module.h — Thu âm qua I2S microphone (INMP441 / SPH0645)
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "driver/i2s_std.h"
#include "config.h"

// ---------- I2S Microphone pins ----------
#define MIC_I2S_WS      15
#define MIC_I2S_SD      13
#define MIC_I2S_SCK     14
#define MIC_I2S_PORT    I2S_NUM_0
#define SAMPLE_RATE     16000

class I2SMic {
public:
    I2SMic();

    /**
     * @brief Khởi tạo driver I2S cho microphone.
     * @return true nếu thành công, false nếu thất bại.
     */
    bool begin();

    /**
     * @brief Đọc mẫu âm thanh từ DMA vào buffer.
     * @param buffer  Con trỏ tới mảng int16_t nhận dữ liệu.
     * @param samples Số lượng mẫu muốn đọc.
     * @return Số lượng mẫu thực tế đã đọc được.
     */
    int read(int16_t* buffer, size_t samples);
};

GPSData sendAudioToServer(const char *serverUrl, int16_t *buffer, size_t sampleCount);

#endif // MICRO_MODULE_H