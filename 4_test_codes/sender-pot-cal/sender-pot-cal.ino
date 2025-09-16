int potPins[] = {36, 39, 35, 34, 32, 33};
int potMin[] = {0,0,0,0,0,0};
int potMax[] ={4095,4095,4095,4095,4095,4095};


void setup() {
  for(int i=0; i<6; i++){
    pinMode(i, INPUT);
  }
  Serial.begin(115200);

}

void loop() {
  for(int i=0; i<6; i++){
    int temp = map(analogRead(potPins[i]), potMin[i], potMax[i], 0, 4095);
    Serial.print(temp);
    Serial.print(", ");
  }
  Serial.println();
  delay(50);
}
