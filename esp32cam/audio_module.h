#pragma once
#include <Arduino.h>
// ============================================================
// audio_module.h — Phát âm thanh qua I2S
// ============================================================
#include <Arduino.h>
/**
 * @brief Khởi tạo Audio với chân I2S và mức âm lượng mặc định.
 */
void audioInit();

/**
 * @brief Gọi trong loop() để duy trì stream âm thanh liên tục.
 */
void audioLoop();

/**
 * @brief Phát âm thanh từ URL HTTP/HTTPS.
 * @param audioUrl URL của file âm thanh cần phát.
 */
void playAudioFromServer(const String& audioUrl);