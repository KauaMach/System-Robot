#define pin_sensor 4
#define led_red 26

int valorpot = 0;  //Variável responsável pelo armazenamento da leitura bruta do potenciometro
int pwm = 0;       //Variável responsável pelo armazenamento do valor convertido pela função map

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  pinMode(led_red, OUTPUT);
  pinMode(pin_sensor, INPUT);
}

void loop() {
  Serial.println(1);
  delay(200);
  // //Serial.println(analogRead((pin_sensor)*3.3/4095));
  // valorpot = analogRead(pin_sensor);//Efetua a leitura do pino analógico
  // Serial.println(valorpot);

  // pwm = map(valorpot, 0, 1023, 0,  255);//Função map() para converter a escala de 0 a 1023 para a escala de 0 a 255


  // Serial.println(pwm);//Imprime valorpot na serial
  // analogWrite(led_red, pwm);//Aciona o LED proporcionalmente à leitura analógica
  // delay(200);//Intervalo de 500 milissegundos
}
