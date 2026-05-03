// ============================================================
// serial_comm.cpp — Giao tiếp UART với Arduino và xử lý dữ liệu
// ============================================================

#include "serial_comm.h"
#include "config.h"
#include <Arduino.h>
#include <HardwareSerial.h>

// UART1 dành riêng để giao tiếp với Arduino
static HardwareSerial ArduinoSerial(1);

void serialCommInit() {
  ArduinoSerial.begin(ARDUINO_BR, SERIAL_8N1,
                      ARDUINO_SERIAL_RX, ARDUINO_SERIAL_TX);
  ArduinoSerial.println("ESP is booting...");
  Serial.println("[Serial] Arduino UART initialized");
}

String serialCommRead() {
  if (ArduinoSerial.available()) {
    return ArduinoSerial.readStringUntil('\n');
  }
  return "";
}

// ------------------------------------------------------------
// parseData — Phân tích định dạng: HEADER|VALUE1,VALUE2
// ------------------------------------------------------------
GPSData parseData(const String& input) {
  char buf[64];
  input.toCharArray(buf, sizeof(buf));
  
  // Khởi tạo giá trị mặc định (không hợp lệ)
  GPSData data = {0.0f, 0.0f, false};

  // Tách Header
  char* token = strtok(buf, "|");
  if (token == NULL) return data;

  String header = String(token);

  if (header == "GPS") {
    // Tách lat và lng
    char* latToken = strtok(NULL, ",");
    char* lngToken = strtok(NULL, ",");

    if (latToken != NULL && lngToken != NULL) {
      data.lat = atof(latToken);
      data.lng = atof(lngToken);
      data.valid = true; // Đánh dấu parse thành công
      
      Serial.printf("[Serial] GPS → lat=%.6f, lng=%.6f\n", data.lat, data.lng);
    } else {
      Serial.println("[Serial] GPS parse error: missing lat/lng");
    }
  }
  
  else {
    Serial.printf("[Serial] Unknown header: %s\n", header.c_str());
  }

  return data;
}