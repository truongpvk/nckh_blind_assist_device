// ============================================================
// esp32cam.ino — Entry point chính (setup & loop)
// ============================================================
//
// Cấu trúc module:
//   config.h         — Định nghĩa toàn bộ pin, baud rate, WiFi credentials
//   camera_module    — Khởi tạo camera, cảm biến, đèn Flash
//   wifi_module      — Kết nối WiFi
//   audio_module     — Phát âm thanh qua I2S (loa)
//   serial_comm      — UART với Arduino, parse GPS / DST
//   micro_module     — Thu âm qua I2S microphone
// ============================================================

#include "config.h"
#include "camera_module.h"
#include "wifi_module.h"
#include "audio_module.h"
#include "serial_comm.h"
#include "micro_module.h"
#include "api_comm.h"

// I2SMic myMic;
const int BUFFER_SIZE = 128;
// int16_t audioBuffer[BUFFER_SIZE]; // Buffer thu âm
int16_t *audioBuffer;  // Buffer thu âm
GPSData destination = { 0.0f, 0.0f, false };
bool inSession = false;
String stream_url = "";

// Khai báo hàm khởi chạy web server từ app_httpd.cpp
void startCameraServer();

// Điều chỉnh tần suất gửi API
unsigned long lastSendTime = 0;
const int SEND_INTERVAL = 200;        // Nghỉ ít nhất 500ms giữa các lần gửi
const float NOISE_THRESHOLD = 500.0;  // Ngưỡng âm thanh (cần điều chỉnh tùy mic)

void logMemory() {
  Serial.printf("[System] Free Heap: %u bytes, Free PSRAM: %u bytes\n",
                ESP.getFreeHeap(), ESP.getFreePsram());
  Serial.printf("Max free block: %d bytes\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

void setup() {
  // --- Debug UART ---
  Serial.begin(DEBUG_BR);
  Serial.setDebugOutput(true);
  Serial.println("\n[Main] Booting ESP32-CAM...");
  logMemory();

  // --- Arduino UART ---
  serialCommInit();
  Serial.println("[Main] Serial comm initialized.");

  // --- Camera ---
  if (!cameraInit()) {
    Serial.println("[Main] Camera init failed. Halting.");
    while (true)
      delay(1000);  // Dừng để dễ debug
  }
  cameraSensorSetup();

  setupLedFlash(LED_GPIO_NUM);

  // --- WiFi ---
  stream_url = wifiInit();

  // Bật server stream camera
  // startCameraServer();
  // Serial.println("[Main] Camera stream server started.");

  // --- Audio (loa) ---
  // audioInit();
  // audioBuffer = (int16_t *)ps_malloc(BUFFER_SIZE * sizeof(int16_t));
  // if (audioBuffer == NULL) {
  //   Serial.println("Lỗi: Không thể cấp phát PSRAM!");
  // }

  // --- Microphone ---
  // if (!myMic.begin()) {
  //   Serial.println("[Main] Mic init failed. Halting.");
  //   while (true)
  //     delay(1000);  // Dừng để dễ debug
  // }

  Serial.println("[Main] Setup complete.");
  logMemory();
}

unsigned long lastMemoryLog = 0;
const unsigned long MEMORY_LOG_INTERVAL = 30000;  // 30 seconds

GPSData ReadCurrentDistance() {
  GPSData data = { 0.0f, 0.0f, false };

  String message = serialCommRead();
  if (message.length() > 0) {
    data = parseData(message);
  }

  return data;
}

void loop() {
  // Duy trì stream âm thanh (loa)
  // audioLoop();

  // Quy trình phát hiện vật thể (Gửi stream -> nhận audio)
  if (stream_url.length() > 0 && millis() - lastSendTime >= SEND_INTERVAL) {
    float distance = 0.0;
    while (distance <= 0.0) distance = ReadCurrentDistance();
    String object_audio = SendImageToServer(String(ROOT_PATH) + String(OBJECT_DETECT_ENDPOINT), distance);
    // if (object_audio.length() > 0) {
    //   playAudioFromServer(object_audio);
    // }
    Serial.println(object_audio);
    lastSendTime = millis();
  }


  // TODO: quy trình hướng dẫn (thu âm -> gửi server -> nhận destination -> gửi current location -> nhận hướng dẫn)
  // int samplesRead = myMic.read(audioBuffer, BUFFER_SIZE);

  // while (samplesRead > 0 && !inSession) {
  //   // Tính toán cường độ âm thanh trung bình trong Buffer
  //   float rms = 0;
  //   for (int i = 0; i < samplesRead; i++) {
  //     rms += sq((float)audioBuffer[i]);
  //   }
  //   rms = sqrt(rms / samplesRead);

  //   // Chỉ gửi nếu âm thanh đủ lớn và đã qua thời gian nghỉ
  //   if (rms <= NOISE_THRESHOLD || millis() - lastSendTime < SEND_INTERVAL)
  //     break;

  //   String uploadUrl = String(ROOT_PATH) + String(GET_DESTINATION_ENDPOINT);
  //   Serial.printf("[Main] Sound detected (RMS: %.2f). Uploading audio to: %s\n", rms, uploadUrl.c_str());

  //   destination = sendAudioToServer(uploadUrl.c_str(), audioBuffer, samplesRead);

  //   if (destination.valid) {
  //     Serial.printf("[Main] Destination received: Lat=%.6f, Lng=%.6f\n", destination.lat, destination.lng);
  //     inSession = true;
  //     lastSendTime = millis();
  //     GPSData currentLocation = ReadCurrentDistance();
  //     Serial.printf("[Main] Current location for route: Lat=%.6f, Lng=%.6f\n", currentLocation.lat, currentLocation.lng);

  //     int routeResponse = CreateRoute(currentLocation, destination);
  //     if (routeResponse == 200) {
  //       Serial.println("[Main] Route initialized successfully.");
  //     } else {
  //       Serial.printf("[Main] ERROR: Failed to initialize route. Code: %d\n", routeResponse);
  //     }
  //   } else {
  //     Serial.println("[Main] Failed to get valid destination from audio.");
  //   }

  //   break;
  // }

  // Periodic memory log
  if (millis() - lastMemoryLog > MEMORY_LOG_INTERVAL) {
    logMemory();
    lastMemoryLog = millis();
  }

  // if (inSession) {
  //   GPSData currentLocation = ReadCurrentDistance();
  //   // Gửi API trả về audio hướng dẫn
  //   String instructionUrl = GetNavigationInstruction(currentLocation);
  //   if (instructionUrl == "complete") {
  //     inSession = false;    // Kết thúc session khi đến đích
  //     instructionUrl = "";  // Không phát âm thanh nào nữa
  //   }

  //   if (instructionUrl.length() > 0) {
  //     playAudioFromServer(instructionUrl);
  //   }
  // }
}

float ReadCurrentDistance() {
  char buf[64];
  input.toCharArray(buf, sizeof(buf));
  
  // Khởi tạo giá trị mặc định (không hợp lệ)
  float distance = 0;

  // Tách Header
  char* token = strtok(buf, "|");
  if (token == NULL) return data;

  String header = String(token);

  if (header == "DST") {
    char* distToken = strtok(NULL, "\n");
    
    if (distToken != NULL) {
      distance = atof(distToken);
      return distance;
    }
  }

  return 0.0f;
}
