#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define TRIG_PIN 10
#define ECHO_PIN 9
#define VIBRA_PIN 8

#define GPS_RX 2
#define GPS_TX 3
#define ESP_RX 4
#define ESP_TX 5
#define GPS_SERIAL Serial3
#define ESP_SERIAL Serial2
#define SIM_SERIAL Serial1

#define ESP_BR 19200
#define GPS_BR 9600
#define SIM_BR 31250
#define DEBUG_BR 4800

// SoftwareSerial GPS_SERIAL(GPS_RX, GPS_TX);
// SoftwareSerial ESP_SERIAL(ESP_RX, ESP_TX);
// SoftwareSerial SIM_SERIAL(7, 6);

const float SOUND_SPEED = 0.034;
const int MAX_DISTANCE = 400;
const int THRESHOLD_CMS = 60;
const unsigned long MAX_DELAY_BETWEEN_MESSAGE = 2000;
TinyGPSPlus gps;

float lat;
float lng;

int index = 0;


void setup() {
  Serial.begin(DEBUG_BR);
  ESP_SERIAL.begin(ESP_BR);
  GPS_SERIAL.begin(GPS_BR);
  SIM_SERIAL.begin(SIM_BR);

  Serial.println("Dang khoi dong...");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(VIBRA_PIN, OUTPUT);
}

void controlMessage(unsigned long maxDelay) {
  unsigned long start = millis();
  bool received = false;

  // Chỉ dùng 1 vòng lặp duy nhất để đợi
  while (millis() - start < maxDelay) {
    if (ESP_SERIAL.available()) {
      String message = ESP_SERIAL.readStringUntil('\n');
      message.trim();
      if (message.length() > 0) {
        // Mạch rung xử lý ở đây
        received = true;
        break;  // Thoát ngay khi nhận được tin
      }
    }
  }

  if (!received) {
    Serial.println("No response from ESP32 (Timeout)");
  }
}
unsigned long start = millis();
void loop() {
  long distance = measure_distance_cm();
  if (distance < THRESHOLD_CMS) analogWrite(VIBRA_PIN, 255);
  else analogWrite(VIBRA_PIN, 0);
  bool allowSend = millis() - start >= 2000;
  
  if (allowSend) {
    ESP_SERIAL.print("DST|");
    ESP_SERIAL.println(distance);
  }
  
  while (GPS_SERIAL.available() > 0) {
    gps.encode(GPS_SERIAL.read());
    break;
  }
  bool test = true;
  // GPS_SERIAL.listen();
  if ((gps.location.isValid() || test) && allowSend) {
    lat = !test || gps.location.isValid() ? gps.location.lat() : 16.05698;
    lng = !test || gps.location.isValid() ? gps.location.lng() : 108.20254;

    ESP_SERIAL.print("GPS|");
    ESP_SERIAL.print(lat, 6);
    ESP_SERIAL.print(",");
    ESP_SERIAL.print(lng, 6);

    start = millis();
  }
}

void send_trigger_pulse() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
}

long read_echo_duration() {
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration;
}

float calculate_distance_from_duration(long duration) {
  float distance = duration * SOUND_SPEED / 2;
  return distance;
}

bool is_distance_valid(float distance) {
  return (distance > 0 && distance <= MAX_DISTANCE);
}

// Đo khoảng cách hoàn chỉnh (hàm chính)
float measure_distance_cm() {
  send_trigger_pulse();
  long duration = read_echo_duration();
  float distance = calculate_distance_from_duration(duration);

  if (!is_distance_valid(distance)) {
    return -1;
  }

  return distance;
}
