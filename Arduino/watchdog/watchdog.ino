#include "esp_system.h"
#include "rom/ets_sys.h"

const int wdtTimeout = 3000;  // timeout watchdog em ms

hw_timer_t *timer = NULL;

int cpuLoad = 10000000;  // controla a carga - número máximo do loop pesado

// ISR que será chamada quando o timer estourar (watchdog)
void IRAM_ATTR resetModule() {
  ets_printf("\nReiniciando por watchdog!\n");
  esp_restart();
}

// Função que simula carga alta na CPU controlada pela variável cpuLoad
void sobrecarregarCPU(int carga) {
  volatile unsigned long i = 0;
  while (i < carga) {
    i++;
    Serial.print(".");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Iniciando setup");

  timer = timerBegin(0, 80, true);  // 80 prescaler → 1 tick = 1 us
  timerAttachInterrupt(timer, &resetModule, true);
  timerAlarmWrite(timer, wdtTimeout * 1000, false);  // timeout em micros
  timerAlarmEnable(timer);
}

void loop() {

  // Alimenta o watchdog (reseta o timer)
  timerWrite(timer, 0);

  // Simula carga de CPU controlada pela variável cpuLoad
  sobrecarregarCPU(cpuLoad);

  // Delay para não resetar muito rápido
  delay(20);
}
