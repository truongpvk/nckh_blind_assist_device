// ============================================================
// wifi_module.cpp — Kết nối và quản lý WiFi
// ============================================================

#include "wifi_module.h"
#include "config.h"
#include <WiFi.h>
#include <Arduino.h>

String wifiInit() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setSleep(false);  // Tắt chế độ ngủ để stream nhanh nhất

    Serial.print("[WiFi] Connecting to ");
    Serial.print(WIFI_SSID);

    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        retryCount++;
        if (retryCount > 40) {  // Timeout 20 giây → restart
            Serial.println("\n[WiFi] Connection failed. Restarting...");
            ESP.restart();
        }
    }

    Serial.println();
    Serial.println("[WiFi] Connected!");
    Serial.printf("[WiFi] Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Gateway:    ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("[WiFi] Stream URL: http://");
    Serial.print(WiFi.localIP());
    Serial.println(":81/stream");

    return "http://" + WiFi.localIP().toString() + ":81/stream";
}