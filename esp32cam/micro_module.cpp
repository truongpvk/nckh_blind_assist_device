// ============================================================
// micro_module.cpp — Thu âm qua I2S microphone (INMP441 / SPH0645)
// ============================================================

#include "micro_module.h"
#include <Arduino.h>


static i2s_chan_handle_t rx_handle = NULL;

I2SMic::I2SMic() {}

bool I2SMic::begin()
{
  // 1. Cấu hình Channel (Bắt buộc dùng Port khác với Loa, ví dụ I2S_NUM_1)
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(MIC_I2S_PORT, I2S_ROLE_MASTER);
  chan_cfg.dma_desc_num = 4;   // Giảm số lượng mô tả DMA (mặc định thường là 6)
  chan_cfg.dma_frame_num = 64; // Giảm kích thước mỗi frame (mặc định thường là 128)

  // Khởi tạo channel mới
  esp_err_t err = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
  if (err != ESP_OK)
  {
    Serial.printf("[Mic] ERROR: Failed to create channel (0x%x)\n", err);
    return false;
  }

  // 2. Cấu hình thông số âm thanh (Standard Mode)
  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
      .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
      .gpio_cfg = {
          .mclk = I2S_GPIO_UNUSED,
          .bclk = (gpio_num_t)MIC_I2S_SCK,
          .ws = (gpio_num_t)MIC_I2S_WS,
          .dout = I2S_GPIO_UNUSED,
          .din = (gpio_num_t)MIC_I2S_SD,
          .invert_flags = {.mclk_inv = false, .bclk_inv = false, .ws_inv = false}},
  };

  // 3. Cài đặt cấu hình và kích hoạt
  i2s_channel_init_std_mode(rx_handle, &std_cfg);
  i2s_channel_enable(rx_handle);

  Serial.println("[Mic] I2S New Driver initialized OK");
  return true;
}

int I2SMic::read(int16_t *buffer, size_t samples)
{
  size_t bytesRead = 0;
  // Sử dụng hàm i2s_channel_read thay cho i2s_read
  esp_err_t result = i2s_channel_read(rx_handle,
                                      buffer,
                                      samples * sizeof(int16_t),
                                      &bytesRead,
                                      portMAX_DELAY);

  if (result != ESP_OK)
  {
    Serial.println("[Mic] ERROR: i2s_channel_read failed");
    return 0;
  }
  return bytesRead / sizeof(int16_t);
}

GPSData sendAudioToServer(const char *serverUrl, int16_t *buffer, size_t sampleCount)
{
  GPSData gpsData = {0.0f, 0.0f, false}; // Giá trị mặc định nếu có lỗi

  if (WiFi.status() != WL_CONNECTED)
    return gpsData;

  HTTPClient http;
  http.begin(serverUrl);

// 1. Set Header về dạng nhị phân thô
  http.addHeader("Content-Type", "application/octet-stream");

  // 2. Tính toán kích thước buffer theo byte
  // Vì buffer là int16_t (2 bytes mỗi sample), nên tổng dung lượng = count * 2
  size_t bytesToSend = sampleCount * sizeof(int16_t);

  // 3. Gửi POST trực tiếp từ vùng nhớ buffer
  // Ép kiểu sang (uint8_t*) để hàm POST hiểu đây là mảng byte thô
  Serial.printf("[Mic] Sending %u bytes of audio data to %s\n", bytesToSend, serverUrl);
  int httpResponseCode = http.POST((uint8_t*)buffer, bytesToSend);

  if (httpResponseCode <= 0)
  {
    Serial.printf("[Mic] ERROR: HTTP POST failed: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
    return gpsData; 
  }

  String response = http.getString();
  Serial.printf("[Mic] Server response code: %d, Response: %s\n", httpResponseCode, response.c_str());

  // 4. Phân tích phản hồi JSON (Giả sử server trả về { "lat": ..., "lng": ... })
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, response);

  // Kiểm tra lỗi khi phân tích JSON
  if (error)
  {
    Serial.printf("[Mic] ERROR: JSON parse failed: %s\n", error.c_str());
    http.end();
    return gpsData; 
  }
  
  // Lấy dữ liệu GPS từ JSON
  if (doc.containsKey("tlat") && doc.containsKey("tlon")) {
    gpsData.lat = doc["tlat"].as<float>();
    gpsData.lng = doc["tlon"].as<float>();
    gpsData.valid = true;
    Serial.printf("[Mic] Parsed Destination: Lat=%.6f, Lng=%.6f\n", gpsData.lat, gpsData.lng);
  } else {
    Serial.println("[Mic] ERROR: JSON missing tlat/tlon fields");
  }

  http.end();
  return gpsData;
}