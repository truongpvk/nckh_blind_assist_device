#pragma once

// ============================================================
// camera_module.h — Khởi tạo camera và cảm biến hình ảnh
// ============================================================

#include "esp_camera.h"
#include <Arduino.h>

/**
 * @brief Khởi tạo camera với cấu hình phù hợp (PSRAM / non-PSRAM).
 * @return true nếu thành công, false nếu thất bại.
 */
bool cameraInit();

/**
 * @brief Cấu hình tham số cảm biến (độ sáng, AWB, flip, v.v.).
 *        Gọi sau khi cameraInit() thành công.
 */
void cameraSensorSetup();

/**
 * @brief Cấu hình chân đèn Flash và tắt mặc định.
 * @param pin Số GPIO của đèn Flash.
 */
void setupLedFlash(int pin);

String SendStreamToServer(String url, String endpoint);

String SendImageToServer(String endpoint, float distance);