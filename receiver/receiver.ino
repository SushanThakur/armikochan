#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"


// channel to look data for
#define CHANNEL 1


// Servo def
Servo servo[6];
int servopins[] = {25, 26, 27, 14, 12, 13};

int servopotpins[] = {36, 39, 34, 35, 32, 33};

int minVal[] = {190, 149, 205, 171, 171, 336};
int maxVal[] = {3530, 3507, 3514, 3376, 3376, 2396};

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
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void readServoPosition(void);
void writeLed(int r, int g, int b);
void removeTrailAtEOF(void);
//void recordMove(void);

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);

  for(int i=0; i<6; i++){
    servo[i].attach(servopins[i]);
    // digitalWrite(servopotpins[i], HIGH);
    pinMode(servopotpins[i], INPUT);
  }

  servoAttached = 1;

  pinMode(pushbttn1, INPUT);
  pinMode(pushbttn2, INPUT);

  if(!SD.begin()){
    Serial.println("SD card initialization failed!");
    writeLed(255, 0, 0);
    while(1){
      //
    }
  }
  writeLed(0, 0, 255);
}

int deleted = 0;

void loop() {
  if(digitalRead(pushbttn1) && !digitalRead(pushbttn2)){   
    if(deleted == 0){
      SD.remove(path);
      deleted = 1;
    }
    for(int i=0; i<6; i++){
      servo[i].detach();
    }
    servoAttached = 0;
    writeLed(255, 0, 0);
    readServoPosition();
    //removeTrailAtEOF();
  }

  if(!digitalRead(pushbttn1) && digitalRead(pushbttn2)){   
    removeTrailAtEOF();
    if(!servoAttached){
      for(int i=0; i<6; i++){
        servo[i].attach(servopins[i]);
      }
    }
    servoAttached = 1;
    writeLed(0, 255, 0);
    readFile(SD, path);
    deleted = 0;
  }
  // Serial.print(analogRead(36));
  // Serial.print(", ");
  // Serial.print(analogRead(39));
  // Serial.println();
  // delay(50);
}


// Functions def

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if(!servoAttached){
    for(int i=0; i<6; i++){
        servo[i].attach(servopins[i]);
    }
    servoAttached = 1;
  }
  for (int i = 0; i < 6; i++) {
    Serial.print(data[i]); Serial.print(", ");
    servo[i].write(data[i]);
  }
  Serial.println();
}

void readServoPosition(void){

    String dataString = "";
    for(int i=0; i<6; i++){
      int temp = analogRead(servopotpins[i]);
      int maptemp = map(temp, minVal[i], maxVal[i], 0, 180);
      dataString += String(maptemp);
      dataString += ",";
      // Serial.print(maptemp);
      // Serial.print(", ");
    }
    //writeFile(SD, "/pos.txt", String(temp).c_str());
    dataString += "\n";
    writeFile(SD, path, dataString);
    // Serial.println();

  writeLed(0, 0, 255);
}

void writeLed(int r, int g, int b){
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

void readFile(fs::FS &fs, const char * path){
  //writeLed(0, 255, 0);
  //Serial.printf("Reading file: %s\n", path);
  // Serial.println();
  // Serial.println("File Started: ");
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  int i = 0;
  //Serial.print("Read from file: ");
  while(file.available()){
    //Serial.write(file.read());
    String buffer = file.readStringUntil(',');
    int rot = buffer.toInt();
    // int rot = (buffer.toInt(), 3450, 190, 0, 180);
    Serial.print(rot);
    // Serial.print("-");
    // Serial.print(i);
    servo[i].write(rot);
    delay(1);
    Serial.print(", ");
    if(i<5){
      i++;
    }
    else{
      i = 0;
      Serial.println();
    }
  }
  file.close();
  writeLed(0, 0, 255);
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
    char* fileContent = new char[fileSize + 1];

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





