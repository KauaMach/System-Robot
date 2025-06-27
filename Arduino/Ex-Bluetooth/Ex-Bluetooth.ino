#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT"); // Nome visível do dispositivo
  Serial.println("Bluetooth iniciado. Agora você pode emparelhar.");
}

void loop() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    Serial.write(c); // Mostra no monitor serial
  }

  if (Serial.available()) {
    SerialBT.write(Serial.read()); // Envia do monitor serial para o Bluetooth
  }
}
