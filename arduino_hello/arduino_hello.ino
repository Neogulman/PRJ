int barcode = SerialUSB;

void setup() {
Serial.begin(9600);
SerialUSB.begin(9600);
}

void loop() {
  int info = digitalRead(barcode);
  Serial.println(barcode);
  Serial.println("Hello Arduino!");
  delay(1000);
}
