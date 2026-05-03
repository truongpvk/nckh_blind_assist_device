// ============================================================
// camera_module.cpp — Khởi tạo camera và cảm biến hình ảnh
// ============================================================

#include "camera_module.h"
#include "config.h"
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

bool cameraInit() {
  // Tắt Brownout Detector để tránh reset liên tục khi camera khởi động
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // OV5640 / OV2640 ổn định nhất ở 20 MHz
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    // VGA (640x480) — nhanh, phù hợp để stream cho YOLO
    Serial.println("[Camera] PSRAM found — using VGA");
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println("[Camera] No PSRAM — using CIF");
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[Camera] Init failed: 0x%x\n", err);
    return false;
  }

  Serial.println("[Camera] Init OK");
  return true;
}

void cameraSensorSetup() {
  // ---------------------------------------------------------------
  // Reset tất cả thông số về mặc định để tránh ảnh tối/mất màu
  // ---------------------------------------------------------------
  sensor_t* s = esp_camera_sensor_get();
  if (!s) {
    Serial.println("[Camera] Sensor get failed");
    return;
  }

  s->set_brightness(s, 0);  // Độ sáng mặc định
  s->set_contrast(s, 0);    // Tương phản mặc định
  s->set_saturation(s, 0);  // Bão hòa mặc định (tránh ảnh đen trắng)
  s->set_sharpness(s, 0);   // Độ sắc nét mặc định
  s->set_ae_level(s, 0);    // Bù phơi sáng về 0

  s->set_exposure_ctrl(s, 1);  // Bật tự động phơi sáng (AEC)
  s->set_aec2(s, 1);           // Bật AEC2 (tốt hơn cho OV5640)
  s->set_whitebal(s, 1);       // Bật tự động cân bằng trắng (AWB)
  s->set_awb_gain(s, 1);       // Bật tự động Gain AWB
  s->set_wb_mode(s, 0);        // Chế độ AWB: Auto

  s->set_hmirror(s, 1);  // Lật ngang
  s->set_vflip(s, 1);    // Lật dọc

  Serial.println("[Camera] Sensor params configured");
}

void setupLedFlash(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);  // Tắt đèn khi khởi động
  Serial.printf("[Camera] LED flash on pin %d — OFF\n", pin);
}

String SendStreamToServer(String url, String endpoint) {
  String audio_stream = "";
  HTTPClient http;
  http.begin(endpoint);

  http.addHeader("Content-Type", "application/json");

  // 1. Chuẩn bị dữ liệu JSON theo API Doc
  // Giả định IP của ESP32 là 192.168.1.100 (bạn nên lấy IP động bằng WiFi.localIP())
  StaticJsonDocument<200> doc;
  doc["device_id"] = "demo";
  doc["stream_url"] = url;

  String requestBody;
  serializeJson(doc, requestBody);

  // 2. Gửi request POST
  Serial.printf("[Camera] Sending stream URL to: %s\n", endpoint.c_str());
  int httpResponseCode = http.POST(requestBody);

  // 3. Xử lý kết quả trả về
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("[Camera] Server response code: %d\n", httpResponseCode);

    if (httpResponseCode == 200) {
      StaticJsonDocument<300> resDoc;
      DeserializationError error = deserializeJson(resDoc, response);

      if (!error) {
        // Lấy link audio stream từ server
        audio_stream = resDoc["audio_stream_url"].as<String>();
        http.end();
        return audio_stream;
      } else {
        Serial.println("[Camera] ERROR: Failed to parse JSON response");
      }
    } else {
      Serial.printf("[Camera] ERROR: Server returned code %d\n", httpResponseCode);
    }
  } else {
    Serial.printf("[Camera] ERROR: POST failed: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  return audio_stream;
}

String SendImageToServer(String endpoint, float distance) {
  // Lấy dữ liệu ảnh
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Lỗi: Không thể lấy dữ liệu từ camera (FAILED_TO_CAPTURE)");
    return "FAILED_TO_CAPTURE";
  }
  
  endpoint += "?distance=" + String(distance);

  HTTPClient http;
  http.begin(endpoint);
  http.setTimeout(30000); 

  // Sử dụng HTTP 1.0 để tránh lỗi chunked
  http.useHTTP10(true);

  // Đổi Content-Type và Accept thành application/json
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");

  // Chuẩn bị payload dạng JSON (ví dụ bạn đã mã hóa base64 trước đó)
  // String jsonPayload = "{\"image_data\":\"" + base64_data + "\"}";
  // int httpResponseCode = http.POST(jsonPayload);

  // Nếu bạn vẫn gửi trực tiếp mảng byte (ví dụ: dùng thư viện để parse json body linh hoạt hơn)
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  String response = "";
  
  if (httpResponseCode > 0) {
    Serial.printf("HTTP Code: %d\n", httpResponseCode);
    
    // Đọc luồng dữ liệu
    WiFiClient *stream = http.getStreamPtr();
    while (stream->available()) {
      response += (char)stream->read();
    }
    
    Serial.print("Phản hồi thô từ server: ");
    Serial.println(response);

    if (response.length() == 0) {
      Serial.println("Lỗi: Phản hồi body trống.");
      http.end();
      esp_camera_fb_return(fb);
      return "EMPTY_RESPONSE";
    }

    // Xử lý JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, response);

    http.end();
    esp_camera_fb_return(fb);

    if (!error) {
      const char* audio_stream = doc["audio_stream"];
      if (audio_stream != nullptr) {
        return String(audio_stream);
      } else {
        return "Không có vật cản!";
      }
    } else {
      Serial.print("Lỗi giải mã JSON: ");
      Serial.println(error.f_str());
      return "";
    }
  } else {
    Serial.printf("Lỗi HTTP Code: %d\n", httpResponseCode);
    http.end();
    esp_camera_fb_return(fb);
    return "";
  }
}