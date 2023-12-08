void setup() {
  pinMode(21, INPUT);
  pinMode(22, INPUT);
  Serial.begin(115200);
}

void loop() {
  Serial.print(digitalRead(21));
  Serial.print(", ");
  Serial.print(digitalRead(22));
  Serial.println();
  delay(20);

}
