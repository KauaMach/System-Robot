#define LED_BUITIN 2
#define but 7

void setup() {
  pinMode(LED_BUITIN, OUTPUT);
  pimMode(ledoff, OUTPUT)

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUITIN, (millis() / 1000 % 2));
  digitalWrite(ledoff, digitalRead(but));


}
