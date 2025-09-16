#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//#include <Ramp.h>

#include <ESPAsyncWebServer.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

#define changeRate 0.05

const char *ssid = "YOUR_SSID_HERE";
const char *password = "YOUR_PASSWORD_HERE";

esp_now_peer_info_t peerInfo;
uint8_t broadcastAddress[] = { 0xCA, 0xC9, 0xA3, 0x60, 0x2B, 0xA5 };
uint8_t data[] = { 0 };

AsyncWebServer server(80);
AsyncWebSocket myWebSocket("/ws");

bool clientConnected = false;


// channel to look data for
#define CHANNEL 1


// Servo def
Servo servo[6];
int servopins[] = { 25, 26, 27, 14, 12, 13 };

int servopotpins[] = { 36, 39, 34, 35, 32, 33 };

int minVal[] = { 190, 132, 205, 171, 140, 2980 };
int maxVal[] = { 3530, 3497, 3514, 3376, 3390, 1900 };

// int minVal[] = {0, 0, 0, 0, 0, 2980};
// int maxVal[] = {4095, 4095, 4095, 4095, 4095, 1900};

int servoAttached = 0;


// Push button pin def
const int pushbttn1 = 21;
const int pushbttn2 = 22;


// LED rgb pin def
const int redwire = 15;
const int greenwire = 2;
const int bluewire = 4;

char path[] = "/pos.txt";

// Functions Dec
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void readServoPosition(void);
void writeLed(int r, int g, int b);
void writeFile(fs::FS &fs, const char *path, const String &message);
void readFile(fs::FS &fs, const char *path);
void removeTrailAtEOF(void);
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
//void recordMove(void);

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());


  esp_now_init();

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  for (int i = 0; i < 6; i++) {
    servo[i].attach(servopins[i]);
    // digitalWrite(servopotpins[i], HIGH);
    pinMode(servopotpins[i], INPUT);
  }

  servoAttached = 1;

  pinMode(pushbttn1, INPUT);
  pinMode(pushbttn2, INPUT);

  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    writeLed(255, 0, 0);
    while (1) {
      //
    }
  }

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"rawliteral(
      <html>
        <head>
          <script>
            var socket = new WebSocket('ws://' + window.location.hostname + '/ws');

            function updateServos() {
              var servoPos = [
                document.getElementById('servo1').value,
                document.getElementById('servo2').value,
                document.getElementById('servo3').value,
                document.getElementById('servo4').value,
                document.getElementById('servo5').value,
                document.getElementById('servo6').value
              ];

              var data = {
                servo1: parseInt(servoPos[0]),
                servo2: parseInt(servoPos[1]),
                servo3: parseInt(servoPos[2]),
                servo4: parseInt(servoPos[3]),
                servo5: parseInt(servoPos[4]),
                servo6: parseInt(servoPos[5])
              };

              socket.send(JSON.stringify(data));
            }
          </script>
        </head>
        <body>
          <h1>Servo Control</h1>
          
          <label for='servo1'>Servo 1</label>
          <input type='range' id='servo1' min='0' max='180' value='90' oninput='updateServos()'><br><br>
          
          <label for='servo2'>Servo 2</label>
          <input type='range' id='servo2' min='0' max='180' value='45' oninput='updateServos()'><br><br>
          
          <label for='servo3'>Servo 3</label>
          <input type='range' id='servo3' min='0' max='180' value='0' oninput='updateServos()'><br><br>
          
          <label for='servo4'>Servo 4</label>
          <input type='range' id='servo4' min='0' max='180' value='40' oninput='updateServos()'><br><br>
          
          <label for='servo5'>Servo 5</label>
          <input type='range' id='servo5' min='0' max='180' value='90' oninput='updateServos()'><br><br>
          
          <label for='servo6'>Servo 6</label>
          <input type='range' id='servo6' min='24' max='102' value='24' oninput='updateServos()'>
        </body>
      </html>
    )rawliteral");
  });

  // Start WebSocket server
  myWebSocket.onEvent(onWebSocketEvent);
  server.addHandler(&myWebSocket);

  // Start the server
  server.begin();

  writeLed(0, 0, 255);
}

int deleted = 0;

void loop() {
  if (digitalRead(pushbttn1) && !digitalRead(pushbttn2)) {
    if (deleted == 0) {
      SD.remove(path);
      deleted = 1;
    }
    for (int i = 0; i < 6; i++) {
      servo[i].detach();
    }
    servoAttached = 0;
    writeLed(255, 0, 0);
    while (1) {
      readServoPosition();
      delay(10);
    }
    //removeTrailAtEOF();
  }

  if (!digitalRead(pushbttn1) && digitalRead(pushbttn2)) {
    removeTrailAtEOF();
    if (!servoAttached) {
      for (int i = 0; i < 6; i++) {
        servo[i].attach(servopins[i]);
      }
    }
    servoAttached = 1;
    writeLed(0, 255, 0);
    deleted = 0;
    while (1) {
      readFile(SD, path);
      delay(20);
    }
  }
  //esp_now_send(broadcastAddress, data, sizeof(data));
  // Serial.print(analogRead(36));
  // Serial.print(", ");
  // Serial.print(analogRead(39));
  // Serial.println();
  // delay(50);
}


// Functions def

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (!servoAttached) {
    for (int i = 0; i < 6; i++) {
      servo[i].attach(servopins[i]);
    }
    servoAttached = 1;
  }
  int temp;
  for (int i = 0; i < 6; i++) {
    Serial.print(data[i]);
    Serial.print(", ");
    if (i == 5) {
      
      if(data[i] == 208){
        temp = 24;
      } else if(data[i] == 232){
        temp = 102;
      } else {
        temp = map(data[i], 0, 180, 24, 102);
      }
      servo[i].write(temp);
    } else {
      servo[i].write(data[i]);
    }
    //servo[i].write(data[i]);
  }
  Serial.println();
}

void readServoPosition(void) {

  String dataString = "";

  for (int i = 0; i < 6; i++) {
    int temp = analogRead(servopotpins[i]);
    int maptemp = map(temp, minVal[i], maxVal[i], i == 5 ? 24 : 0, i == 5 ? 102 : 180);
    if (i == 5) {
      if (maptemp > 120) {
        maptemp = 120;
      } else if (maptemp < 14) {
        maptemp = 24;
      }
    } else {
      if (maptemp > 180) {
        maptemp = 180;
      } else if (maptemp < 0) {
        maptemp = 0;
      }
    }
    dataString += String(maptemp);
    dataString += ",";
    // Serial.print(maptemp);
    // Serial.print(", ");
  }

  //writeFile(SD, "/pos.txt", String(temp).c_str());

  dataString += "\n";
  writeFile(SD, path, dataString);
  // Serial.println();

  //writeLed(0, 0, 255);
}

void writeLed(int r, int g, int b) {
  analogWrite(redwire, r);
  analogWrite(greenwire, g);
  analogWrite(bluewire, b);
}

void writeFile(fs::FS &fs, const char *path, const String &message) {
  //Serial.printf("Writing file: %s\n", path);
  //writeLed(255, 0, 0);
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.print("Failed to open file for writing");
    return;
  }
  if (file.print(message.c_str())) {
    //Serial.print("File written");
  } else {
    Serial.print("Write failed");
  }
  file.close();
  //delay(50);
}

void readFile(fs::FS &fs, const char *path) {
  //writeLed(0, 255, 0);
  //Serial.printf("Reading file: %s\n", path);
  // Serial.println();
  // Serial.println("File Started: ");
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  int i = 0;
  //Serial.print("Read from file: ");
  while (file.available()) {
    //Serial.write(file.read());
    String buffer = file.readStringUntil(',');
    int rot = buffer.toInt();
    // int rot = (buffer.toInt(), 3450, 190, 0, 180);
    Serial.print(rot);
    // Serial.print("-");
    // Serial.print(i);
    servo[i].write(rot);  //////////////////////////////////////////////
    delay(2);
    Serial.print(", ");
    if (i < 5) {
      i++;
    } else {
      i = 0;
      Serial.println();
    }
  }
  file.close();
  //writeLed(0, 0, 255);
}

// This function was written with the help of ChatGPT
void removeTrailAtEOF() {
  // Open the file in read mode
  File file = SD.open(path, FILE_READ);

  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Get the file size
  int fileSize = file.size();

  // If the file is not empty
  if (fileSize > 0) {
    // Allocate a buffer to store the content of the file
    char *fileContent = new char[fileSize + 1];

    // Read the entire content of the file
    file.readBytes(fileContent, fileSize);

    // Null-terminate the string
    fileContent[fileSize] = '\0';

    // Close the file in read mode
    file.close();

    // Modify the string to remove the trailing comma
    if (fileSize >= 2 && fileContent[fileSize - 2] == ',') {
      fileContent[fileSize - 2] = '\0';  // Remove the trailing comma
    }

    // Open the file again in write mode to truncate it
    File outFile = SD.open(path, FILE_WRITE);

    if (outFile) {
      // Write the modified string back to the file
      outFile.print(fileContent);

      // Close the output file
      outFile.close();

      //Serial.println("Trailing comma removed successfully.");
    } else {
      //Serial.println("Failed to open output file for writing.");
    }

    // Deallocate the buffer
    delete[] fileContent;
  } else {
    //Serial.println("File is empty.");
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
    clientConnected = true;
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
    clientConnected = false;
  } else if (type == WS_EVT_DATA && clientConnected) {
    // Parse JSON data from the WebSocket message
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (const char *)data);

    // Update servo positions
    servo[0].write(doc["servo1"]);
    servo[1].write(doc["servo2"]);
    servo[2].write(doc["servo3"]);
    servo[3].write(doc["servo4"]);
    servo[4].write(doc["servo5"]);
    servo[5].write(doc["servo6"]);
  }
}