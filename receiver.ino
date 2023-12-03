#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

Servo servo[6];
int servopins[] = {25, 26, 27, 14, 12, 13};

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);

  for(int i=0; i<6; i++){
    servo[i].attach(servopins[i]);
  }
}

// Callback when data is received from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  for (int i = 0; i < 6; i++) {
    Serial.print(data[i]); Serial.print(", ");
    servo[i].write(data[i]);
  }
  Serial.println();
}

void loop() {
  // Your main loop code (if any) goes here
}
