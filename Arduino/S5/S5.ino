#include <esp_timer.h>
#include "esp_system.h"   //funções básicas do sistema ESP
#include "rom/ets_sys.h"  //funções do sistema de tempo real do ESP

// Definição de pinos
#define ONBOARD_LED 2   // LED da placa
#define BUTTON_PIN 15   // Botão com pull-up interno
#define DIGITAL_LED 26  // LED digital
#define POT_PIN 4       // Entrada analógica
#define PWM_LED 32      // LED com PWM


//-----------------------
// Função Blink
hw_timer_t *timerBlink = NULL;  // Timer para blink
void IRAM_ATTR blinkISR() {
  digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));  // inverte o estado do LED
}

//-----------------------
// Função Botão
volatile bool ledState = false;
volatile uint64_t lastInterruptTime = 0;
void IRAM_ATTR botaoISR() {
  uint64_t now = esp_timer_get_time();  // tempo atual em micros

  if (now - lastInterruptTime > 50000) {  // debounce 20ms (20000 micros)
    ledState = !ledState;
    digitalWrite(DIGITAL_LED, ledState);
    lastInterruptTime = now;
  }
}

//------------------------
// Função Watchdog
const int wdtTimeout = 3000;       // Timeout do watchdog em ms (3 segundos)
hw_timer_t *timerWatchdog = NULL;  // Ponteiro para timer hardware
void IRAM_ATTR resetModule() {
  ets_printf("\nReiniciando por watchdog!\n");
  esp_restart();  //para reiniciar o ESP.
}

//-----------------------
// Variaveis Globais de Controle
int quantidade = 0;  // controle da carga de CPU 34651 REINICIA
int brilhoPWM = 0;
bool statusLog = false;
bool menuAtivo = false;
int estadoMenu = 0;
String entradaSerial = "";


//-------------------------------
//Definição de funções
void sobrecarregar(int carga);
void brilhoLedPot();
void processarEntradaSerial();
void tratarMenu(String comando);
void log(String msg);

void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(DIGITAL_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PWM_LED, OUTPUT);

  // Blink com timer
  timerBlink = timerBegin(0, 80, true);  // 80MHz / 80 = 1MHz → 1 tick = 1 microsegundo
  timerAttachInterrupt(timerBlink, &blinkISR, true);
  timerAlarmWrite(timerBlink, 500000, true);  //alarme para 500.000 us = 500 ms
  timerAlarmEnable(timerBlink);

  // Interrupção do botão
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), botaoISR, FALLING);

  // Configura Watchdog
  timerWatchdog = timerBegin(1, 80, true);  // 80 prescaler → 1 tick = 1 us
  timerAttachInterrupt(timerWatchdog, &resetModule, true);
  timerAlarmWrite(timerWatchdog, wdtTimeout * 1000, false);  // timeout em micros
  timerAlarmEnable(timerWatchdog);
}

void loop() {
  // Serial.println("\nRodando loop principal");

  timerWrite(timerWatchdog, 0);  // Alimenta o watchdog (reseta o timer)
  brilhoLedPot();                // PWM com potenciômetro
  processarEntradaSerial();      // Menu interativo
  sobrecarregar(quantidade);     // Função de carga
  delay(20);
}

//------------------------
// Função de carga simulada
void sobrecarregar(int carga) {
  volatile unsigned long i = 0;
  while (i < carga) {
    i++;
    Serial.print(".");
    yield();
  }
}

// void sobrecarregarCPU() {
//   // Loop infinito de cálculo sem delay - alta carga na CPU
//   volatile unsigned long i = 0;
//   while (true) {
//     i++;
//     if (i % 1000000 == 0) {
//       Serial.println("Processando");
//       Serial.println(i);
//     }
//   }
// }

//------------------------
// Função: Leitura analógica e controle PWM
void brilhoLedPot() {
  static int ultimoValor = 0;
  int valorPOT = analogRead(POT_PIN);
  if (abs(valorPOT - ultimoValor) > 10) {
    brilhoPWM = map(valorPOT, 0, 4095, 0, 255);
    analogWrite(PWM_LED, brilhoPWM);
    log("\nPotenciômetro: " + String(valorPOT) + " | PWM: " + String(brilhoPWM));
    log("");
    ultimoValor = valorPOT;
  }
}

//------------------------
// Função: Processa a chamada do menu  via Serial
void processarEntradaSerial() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      entradaSerial.trim();
      if (!menuAtivo && entradaSerial.equalsIgnoreCase("MENU")) {
        menuAtivo = true;
        estadoMenu = 0;
        //statusLog = false;
      } else if (menuAtivo) {
        tratarMenu(entradaSerial);
      }
      entradaSerial = "";
    } else {
      entradaSerial += c;
    }
  }
}

//------------------------
// Menu interativo via Serial
void tratarMenu(String comando) {
  switch (estadoMenu) {
    case 0:
      Serial.println("====== MENU ======");
      Serial.println("1 - Ligar LED Digital");
      Serial.println("2 - Desligar LED Digital");
      Serial.println("3 - Ajustar brilho PWM (0-255)");
      Serial.println("4 - Ajustar quantidade da função carga");
      Serial.println("5 - Ativar log");
      Serial.println("6 - Desativar log");
      Serial.println("==================");
      Serial.println("Digite a opção desejada: ");
      estadoMenu = 1;
      break;

    case 1:
      if (comando == "1") {
        digitalWrite(DIGITAL_LED, HIGH);
        Serial.println("LED Digital LIGADO");
        menuAtivo = false;
      } else if (comando == "2") {
        digitalWrite(DIGITAL_LED, LOW);
        Serial.println("LED Digital DESLIGADO");
        menuAtivo = false;
      } else if (comando == "3") {
        Serial.println("Digite o valor do brilho (0-255): ");
        estadoMenu = 2;
      } else if (comando == "4") {
        Serial.println("Digite a quantidade para a função carga: ");
        estadoMenu = 3;
      } else if (comando == "5") {
        statusLog = true;
        Serial.println("Log ATIVADO");
        menuAtivo = false;
      } else if (comando == "6") {
        statusLog = false;
        Serial.println("Log DESATIVADO");
        menuAtivo = false;
      } else {
        Serial.println("Opção inválida. Tente novamente.");
        estadoMenu = 0;
      }
      break;

    case 2:  // PWM
      brilhoPWM = constrain(comando.toInt(), 0, 255);
      analogWrite(PWM_LED, brilhoPWM);
      Serial.print("Brilho PWM ajustado para: ");
      Serial.println(brilhoPWM);
      menuAtivo = false;
      break;

    case 3:  // carga
      {
        int qnt = comando.toInt();
        if (qnt > 0) {
          quantidade = qnt;
          Serial.print("Quantidade da função carga ajustada para: ");
          Serial.println(quantidade);
        } else {
          Serial.println("Valor inválido.");
        }
        menuAtivo = false;
      }
      break;
  }
}


//------------------------
// Log condicional
void log(String msg) {
  if (statusLog) Serial.print(msg);
}
