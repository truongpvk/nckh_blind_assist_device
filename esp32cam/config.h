#pragma once

// ============================================================
// config.h — Toàn bộ cấu hình phần cứng và mạng
// ============================================================


// ---------- Camera pins (AI-Thinker ESP32-CAM) ----------
// #define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_ESP_EYE  // Has PSRAM

#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

// ---------- LED ----------
#define LED_GPIO_NUM        4   // Đèn Flash
#define LED_BUILTIN_PIN    33

// ---------- I2S Audio ----------
#define I2S_BCLK   14
#define I2S_LRC    15
#define I2S_DOUT    1

// ---------- UART giao tiếp Arduino ----------
#define ARDUINO_SERIAL_RX  12
#define ARDUINO_SERIAL_TX  13
#define DEBUG_BR        115200
#define ARDUINO_BR       19200

// ---------- WiFi ----------
// Thay thông tin mạng phù hợp tại đây
// #define WIFI_SSID      "Khanh Truong"
// #define WIFI_PASSWORD  "tin18102004"

#define WIFI_SSID      "HAN PHUONG"
#define WIFI_PASSWORD  "supiphuonghan"

#define ROOT_PATH "https://superstrenuous-vanesa-prepositively.ngrok-free.dev"
#define OBJECT_DETECT_ENDPOINT "/vision/analyze-image"
#define GET_DESTINATION_ENDPOINT "/nav/init-navigation"
#define INSTRUCT_ENDPOINT "/nav/instruct"
#define ROUTE_ENDPOINT "/nav/route"

struct GPSData {
    float lat;
    float lng;
    bool valid; // Dùng để kiểm tra việc parse có thành công hay không
};