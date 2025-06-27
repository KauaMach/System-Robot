#include "BluetoothSerial.h"


char readBluetooth = ' ';  //Variável para armazenar o caractere recebido


#define USE_PIN
const char *pin = "1234";

String device_name = "ROBOTTEEN";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  delay(1000);
  SerialBT.begin(device_name);
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  #ifdef USE_PIN
  SerialBT.setPin(pin, strlen(pin));
  Serial.println("Using PIN");
  #endif

  pinMode(2, OUTPUT);  //colocamos o LED na porta 2 do nosso esp, ou em qualquer outra porta que estiver disponível.
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    readBluetooth = SerialBT.read();
    Serial.write(readBluetooth);
  }

  switch (readBluetooth)  //Verifica se o caractere recebido é igual a algum dos listados abaixo
  {
    case 'A':  // up
      Serial.println("A");
      digitalWrite(2, HIGH);  //Aqui liga o LED
      break;

    case 'B':  // up
      Serial.println("B");
      digitalWrite(2, LOW);  //Aqui desliga o LED
      break;

    //ESQUERDA
    case 'C':                 // up
      Serial.println("C");    //caso queira fazer a animação piscar sempre, só mudar o "C" maiúsculo, e colocar o "c" minusculo
      digitalWrite(2, HIGH);  //Aqui faz a animação com os LEDs, que faria piscar
      delay(500);
      digitalWrite(2, LOW);
      delay(500);
      break;

    case 'D':  // up
      Serial.println("D");
      break;
  }
  delay(20);
}