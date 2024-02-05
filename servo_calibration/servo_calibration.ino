#include <ESP32Servo.h>

Servo servo[6];
int servopins[] = {25, 26, 27, 14, 12, 13};

int servopotpins[] = {36, 39, 34, 35, 32, 33};

const int ser = 1;
int long avg=0;

void checkMin(void);
void checkMax(void);

void setup() {
  Serial.begin(115200);

  for(int i=0; i<6; i++){
    servo[i].attach(servopins[i]);
    pinMode(servopotpins[i], INPUT);
  }

  avg = 0;
  checkMin();
  avg = 0;
  delay(1000);
  checkMax();

}

void loop() {
  

}

void checkMax(void){
  // for(int i=0; i<10; i++){
  //   servo[ser].write(0);
  //   int temp = analogRead(servopotpins[ser]);
  //   avg += temp;
  //   delay(1000);
  //   servo[ser].write(180);
  //   delay(1000);
  // }
  // avg = avg / 10;
  servo[ser].write(0);
  delay(1500);
  int avg = analogRead(servopotpins[ser]);
  Serial.print("Max = ");
  Serial.println(avg);
}

void checkMin(void){
  // for(int i=0; i<10; i++){
  //   servo[ser].write(180);
  //   int temp = analogRead(servopotpins[ser]);
  //   avg += temp;
  //   delay(1000);
  //   servo[ser].write(0);
  //   delay(1000);
  // }
  // avg = avg / 10;
  servo[ser].write(180);
  delay(1500);
  int avg = analogRead(servopotpins[ser]);
  Serial.print("Min = ");
  Serial.println(avg);
}
