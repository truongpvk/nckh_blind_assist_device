// ============================================================
// audio_module.cpp — Phát âm thanh qua I2S
// ============================================================

#include "audio_module.h"
#include "config.h"
#include "Audio.h"
#include <Arduino.h>

// Đối tượng Audio dùng nội bộ trong module này
static Audio* audio = nullptr;

void audioInit() {
    if (audio == nullptr) {
        audio = new Audio();
    }
    audio->setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio->setVolume(15);  // Mức âm lượng 0–21
    Serial.println("[Audio] I2S initialized, volume = 15");
}

void audioLoop() {
    if (audio) {
        audio->loop();
    }
}

void playAudioFromServer(const String& audioUrl) {
    if (!audio) return;
    
    Serial.print("[Audio] Playing from: ");
    Serial.println(audioUrl);

    // Dừng bài đang phát trước khi nạp luồng mới
    audio->stopSong();

    bool ok = audio->connecttohost(audioUrl.c_str());
    if (!ok) {
        Serial.println("[Audio] ERROR: Cannot connect to audio stream!");
    }
}