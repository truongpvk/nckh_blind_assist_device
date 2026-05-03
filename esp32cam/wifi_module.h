#pragma once
#include <Arduino.h>
// ============================================================
// wifi_module.h — Kết nối và quản lý WiFi
// ============================================================

/**
 * @brief Kết nối WiFi với thông tin trong config.h.
 *        Tự động restart ESP nếu không kết nối được sau 20 giây.
 */
String wifiInit();