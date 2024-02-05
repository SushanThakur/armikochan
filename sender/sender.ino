#include <esp_now.h>
#include <WiFi.h>

#define changeRate 0.05

esp_now_peer_info_t receiver;
uint8_t esp32receiver[] = {0xB0, 0xA7, 0x32, 0xF3, 0x64, 0x30};

int potPins[] = {36, 39, 35, 34, 32, 33};

int potMin[] = {0,250,3450,4095,2440,0};  
int potMax[] ={2700,2700,1420,1360,0,528};

// LED rgb pin def
const int redwire = 15;
const int greenwire = 2;
const int bluewire = 4;

uint8_t data[6];

void writeLed(int, int, int);

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  // Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup(){

  WiFi.mode(WIFI_STA);  
  WiFi.disconnect();
  esp_now_init();
  memcpy(receiver.peer_addr, esp32receiver, 6);
  receiver.channel = 1;
  receiver.encrypt = false;
  esp_now_add_peer(&receiver);
  esp_now_register_send_cb(OnDataSent);

  for(int i=0; i<6; i++){
    pinMode(i, INPUT);
  }
  Serial.begin(115200);

  writeLed(0, 0, 255);
}

int pre;
int current;

void loop(){
  for(int i=0; i<6; i++){
    //int temp = map(analogRead(potPins[i]), potMin[i], potMax[i], 0, 180);

    //Serial.print("temp = "); Serial.print(temp);
    //data[i] = temp;
    //Serial.print("t=");Serial.print()

    pre = data[i];
    // int temp = map((i == 0 || i == 1 || i == 4)?(analogRead(potPins[i]) < potMin[i] ? potMin[i] : analogRead(potPins[i])):analogRead(potPins[i]), potMin[i], potMax[i], 0, 200); //0 1 4

    int temp = map(analogRead(potPins[i]), potMin[i], potMax[i], 0, 200);
    
    //(i == 0 || i == 1 || i == 4)?(analogRead(potPins[i]) < potMin[i] ? potMin[i] : analogRead(potPins[i])):analogRead(potPin[i])
    
    
    current = (temp * changeRate) + (pre * (1-changeRate));

    pre = current;

    data[i] = current;
    if(data[i]>180){
      data[i] = 180;
    } else if(data[i]<0){
      data[i] = 0;
    }

    // if(i==5){
    //   data[i] = map(data[i], 0, 180, 0, 102);
    // }

    // if(abs(data[i]-temp) > 2){
    //   data[i] = temp;
    // } else if(temp == 0) {
    //   data[i] = 0;
    // } else if(temp == 180){
    //   data[i] = 180;
    // }


    //Serial.print(" Data = ");
    Serial.print(data[i]); Serial.print(", ");
  }
  Serial.println();
  esp_now_send(esp32receiver, data, sizeof(data));
  //delay(200);
}

void writeLed(int r, int g, int b){
  analogWrite(redwire, r);
  analogWrite(greenwire, g);
  analogWrite(bluewire, b);
}
