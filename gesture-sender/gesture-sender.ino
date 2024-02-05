///#include <LiquidCrystal.h>

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include "LiquidCrystal_I2C.h"

uint8_t esp32receiver[] = {0xB0, 0xA7, 0x32, 0xF3, 0x64, 0x31};

MPU6050 mpu;

int servoPos[] = {90, 45, 0, 40, 90, 30};
uint8_t data[6];

void OnDataSent(uint8_t *mac_addr, uint8_t status) {
    // if (status == 0) {
    //     Serial.println("Message sent successfully");
    // } else {
    //     Serial.println("Message failed to send");
    // }
}

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != 0) {
        Serial.println("ESP-NOW initialization failed");
        return;
    }

    pinMode(D6, INPUT);
    digitalWrite(D6, HIGH);

  //   WiFi.mode(WIFI_STA);
  //   WiFi.disconnect();
  //   esp_now_init();

  //   memcpy(receiver.peer_addr, esp32receiver, 6);
  //   receiver.channel = 1;
  //   receiver.encrypt = false;

  //   esp_now_add_peer(receiver.peer_addr, ESP_NOW_ROLE_COMBO, receiver.channel, receiver.lmk, 16);
  //   esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(OnDataSent));

  // //esp_now_add_peer(&receiver);
  // //esp_now_register_send_cb(OnDataSent);

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_add_peer(esp32receiver, ESP_NOW_ROLE_COMBO, 1, NULL, 0);


  pinMode(D7,INPUT);

  Wire.begin();
  delay(500);
  mpu.initialize();

  lcd.init();                    
  lcd.backlight();

}

void loop() {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    float accelerationX = ax / 16384.0;
    float accelerationY = ay / 16384.0;
    float accelerationZ = az / 16384.0;

    for (int i = 0; i < 6; i++) {
        if (i == 0 || i==4) {
            servoPos[i] = servoPos[i] + (-accelerationX / 10 * 45);
        } else if (i == 1 || i == 2 || i == 3) {
            servoPos[i] = servoPos[i] + (-accelerationY / 10 * 45);
        } 
        if (servoPos[i] >= 180) {
            servoPos[i] = 180;
        } else if (servoPos[i] <= 0) {
            servoPos[i] = 0;
        }
        if(i==5){
          servoPos[i] = (digitalRead(D7)==1)?232:208;
        }
    }

    for (int i = 0; i < 6; i++) {
        data[i] = servoPos[i];
        //data[i] = map(servoPos[i],0,180,180,0);
        Serial.print(data[i]);
        Serial.print(", ");
    }
    Serial.println();
    lcd.clear();

    esp_now_send(esp32receiver, data, sizeof(data));
    lcd.setCursor(0,0);
    lcd.print(servoPos[0]);
    lcd.setCursor(4,0);
    lcd.print(servoPos[1]);
    lcd.setCursor(8,0);
    lcd.print(servoPos[2]);
    lcd.setCursor(0,1);
    lcd.print(servoPos[3]);
    lcd.setCursor(4,1);
    lcd.print(servoPos[4]);
    lcd.setCursor(8,1);
    lcd.print(servoPos[5]);
    delay(20);
}

void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len) {
  Serial.print(*data);
  Serial.println();
}
