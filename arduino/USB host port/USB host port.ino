#include <Arduino.h>
void setup() {
  Serial.begin(9600);
  SerialUSB.begin(4800);
  //while(!SerialUSB.available() ) {
  //  delay(500);
  //}
}

void loop() {
  //int info = SerialUSB.read();
  //Serial.println(info);
  Serial.println(SerialUSB.readString());
  Serial.println("Hello Arduino!");
  delay(1000);
}
