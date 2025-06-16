#include <esp_timer.h>
#include "esp_system.h"   //funções básicas do sistema ESP
#include "rom/ets_sys.h"  //funções do sistema de tempo real do ESP

// Definição de pinos
#define ONBOARD_LED 2   // LED da placa
#define BUTTON_PIN 15   // Botão com pull-up interno
#define DIGITAL_LED 26  // LED digital

// Variaveis Globais de Controle
int quantidade = 346 ;  // controla a carga - número máximo do loop pesado

//------------------------
// Função Blink
hw_timer_t *timerBlink = NULL;  // Timer para blink
void IRAM_ATTR blinkISR() {
  digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));  // inverte o estado do LED
}

//------------------------
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
const int wdtTimeout = 3000;  // Timeout do watchdog em ms (3 segundos)
hw_timer_t *timerWatchdog  = NULL;     // Ponteiro para timer hardware

void IRAM_ATTR resetModule() {
  ets_printf("\nReiniciando por watchdog!\n");
  esp_restart();  //para reiniciar o ESP.
}


//-------------------------------
//Definição de funções
void sobrecarregar();


void setup() {
  Serial.begin(115200);

  // Configura LED onboard para piscar com timer
  pinMode(ONBOARD_LED, OUTPUT);
  // Configura o timer 0 com prescaler de 80 (1us por tick)
  timerBlink = timerBegin(0, 80, true);  // 80MHz / 80 = 1MHz → 1 tick = 1 microsegundo
  timerAttachInterrupt(timerBlink, &blinkISR, true);
  timerAlarmWrite(timerBlink, 500000, true);  //alarme para 500.000 us = 500 ms
  timerAlarmEnable(timerBlink);

  // Configura LED digital e botão
  pinMode(DIGITAL_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), botaoISR, FALLING);


  // Configura Watchdog
  timerWatchdog = timerBegin(1, 80, true);  // 80 prescaler → 1 tick = 1 us
  timerAttachInterrupt(timerWatchdog , &resetModule, true);
  timerAlarmWrite(timerWatchdog, wdtTimeout * 1000, false);  // timeout em micros
  timerAlarmEnable(timerWatchdog);
}

void loop() {
  Serial.println("\nRodando loop principal");
  timerWrite(timerWatchdog, 0);  // Alimenta o watchdog (reseta o timer)
  sobrecarregar(quantidade);     // Simula carga de CPU controlada pela variável quantidade
  delay(20);
}

//------------------------
// Função que simula carga alta na CPU controlada pela variável cpuLoad
void sobrecarregar(int carga) {
  volatile unsigned long i = 0;
  while (i < carga) {
    i++;
    Serial.print(".");
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