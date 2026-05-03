#pragma once

// ============================================================
// serial_comm.h — Giao tiếp UART với Arduino và xử lý dữ liệu
// ============================================================

#include <Arduino.h>
#include "config.h"

/**
 * @brief Khởi tạo UART phần cứng để giao tiếp với Arduino.
 */
void serialCommInit();

/**
 * @brief Đọc một dòng dữ liệu từ Arduino (nếu có).
 * @return Chuỗi nhận được, hoặc "" nếu không có dữ liệu.
 */
String serialCommRead();

/**
 * @brief Phân tích chuỗi dữ liệu theo định dạng HEADER|DATA.
 *        Các header hỗ trợ: "GPS", "DST".
 * @param input Chuỗi cần phân tích.
 */
GPSData parseData(const String& input);