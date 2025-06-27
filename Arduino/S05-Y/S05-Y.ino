#include <Arduino.h>
#include <esp_task_wdt.h>
#include "esp_system.h"

#define BOTAO 15
#define LED_VERDE 26
#define LED_AZUL 32
#define LED_ONBOARD 2
#define POT 4

#define TEMPO_DEBOUNCE 50
#define TEMPO_BLINK_US 100000  // 100 ms em microssegundos

//-----------------------------------------------
struct Estado {
  bool statusLog = true;
  int valorLedAzul = 0;
  String fonteUltimaAtualizacao = "nenhuma";
  int ultimoPotMapeado = -1;

  bool cargaAtiva = false;
  int quantidadeCarga = 0;
  int contadorCarga = 0;
  unsigned long ultimoTempoCarga = 0;

  bool blinkAtivo = false;
  bool estadoOnBoard = LOW;
  unsigned long ultimoTempoBlink = 0;

  int statusBotao = LOW;
  int ultimoStatusBotao = LOW;
  unsigned long tempoUltimoDebounce = 0;

  bool menuAtivo = false;
  int estadoMenu = 0;
};

//--------------------------------------------
Estado estado;
String entradaSerial = "";

//--------------------------------------------
hw_timer_t *blinkTimer = NULL;
void IRAM_ATTR toggleBlink() {
  if (estado.blinkAtivo) {
    estado.estadoOnBoard = !estado.estadoOnBoard;
    digitalWrite(LED_ONBOARD, estado.estadoOnBoard);
  }
}

//--------------------------------------------
const int wdtTimeout = 3000;
hw_timer_t *timerWatchdog = NULL;
void IRAM_ATTR resetModule() {
  ets_printf("\nReiniciando por watchdog!\n");
  esp_restart();
}

//--------------------------------------------
void IRAM_ATTR botaoISR() {
  bool botaoPressionado = digitalRead(BOTAO) == LOW;
  digitalWrite(LED_VERDE, botaoPressionado ? LOW : HIGH);
  log("Botão " + String(botaoPressionado ? "pressionado" : "liberado"));
}

//--------------------------------------------
void log(String msg) {
  if (estado.statusLog) {
    Serial.println("[LOG] " + msg);
  }
}

void carga() {
  volatile unsigned long i = 0;
  Serial.println("Iniciando carga...");
  while (i < estado.quantidadeCarga) {
    i++;
    Serial.print(".");
    yield();
  }
  Serial.println("\nCarga finalizada.");
}
//--------------------------------------------
void menu() {
  Serial.println("\n=== MENU ===");
  Serial.println("1 - Enviar número para controle de carga");
  Serial.println("2 - Enviar texto para ligar/desligar LED Verde");
  Serial.println("3 - Ativar carga (quantidade de ciclos)");
  Serial.println("4 - Ativar/desativar LOG");
  Serial.println("5 - Ativar/desativar Blink LED Onboard");
  Serial.println("6 - Configurar Watchdog (tempo em segundos)");
  Serial.println("Digite uma opção:");
}

bool aguardarEntradaSerial() {
  unsigned long inicio = millis();
  while (!Serial.available()) {
    timerWrite(timerWatchdog, 0);
    if (millis() - inicio > 10000) {
      Serial.println("Timeout de entrada serial.");
      return false;
    }
  }
  return true;
}

void executarOpcao(int opcao) {
  log("Executando opção: " + String(opcao));
  switch (opcao) {
    case 1:
      Serial.println("Digite um número entre 0 e 255:");
      if (aguardarEntradaSerial()) {
        String entrada = Serial.readStringUntil('\n');
        entrada.trim();
        int valor = entrada.toInt();
        estado.valorLedAzul = constrain(valor, 0, 255);
        estado.fonteUltimaAtualizacao = "serial";
        log("Valor LED Azul via serial: " + String(estado.valorLedAzul));
      }
      break;

    case 2:
      Serial.println("Digite um texto (ex: 'asd'):");
      if (aguardarEntradaSerial()) {
        String entrada = Serial.readStringUntil('\n');
        entrada.trim();
        log("Texto recebido: " + entrada);
        if (entrada == "asd") {
          digitalWrite(LED_VERDE, HIGH);
          log("LED Verde LIGADO");
        } else {
          digitalWrite(LED_VERDE, LOW);
          log("LED Verde DESLIGADO");
        }
      }
      break;

    case 3:
      Serial.println("Digite a quantidade de ciclos:");
      if (aguardarEntradaSerial()) {
        estado.quantidadeCarga = Serial.parseInt();
        log("Carga definida para: " + String(estado.quantidadeCarga) + " ciclos");
      }
      break;

    case 4:
      estado.statusLog = !estado.statusLog;
      Serial.println(estado.statusLog ? "LOG ATIVADO" : "LOG DESATIVADO");
      break;

    case 5:
      estado.blinkAtivo = !estado.blinkAtivo;
      if (!estado.blinkAtivo) {
        digitalWrite(LED_ONBOARD, LOW);
      }
      Serial.println(estado.blinkAtivo ? "Blink ATIVADO" : "Blink DESATIVADO");
      break;

    case 6:
      Serial.println("Digite o tempo do watchdog em segundos:");
      if (aguardarEntradaSerial()) {
        int tempo = Serial.parseInt();
        if (tempo >= 1 && tempo <= 30) {
          timerAlarmWrite(timerWatchdog, tempo * 1000 * 1000, false);
          Serial.println("Watchdog atualizado para " + String(tempo) + " segundos.");
          log("Watchdog configurado para " + String(tempo) + " segundos");
        } else {
          Serial.println("Tempo inválido. Digite um valor entre 1 e 30 segundos.");
        }
      }
      break;

    default:
      Serial.println("Opção inválida.");
  }
}

void processarEntradaSerial() {
  if (Serial.available()) {
    char c = Serial.read();
    timerWrite(timerWatchdog, 0);
    log("Caractere recebido: '" + String(c) + "'");

    if (c == '\n') {
      entradaSerial.trim();
      log("Comando completo recebido: " + entradaSerial);
      if (!estado.menuAtivo && entradaSerial.equalsIgnoreCase("MENU")) {
        estado.menuAtivo = true;
        estado.estadoMenu = 0;
        menu();
      } else if (estado.menuAtivo) {
        int opcao = entradaSerial.toInt();
        estado.menuAtivo = false;
        Serial.println("Opção escolhida: " + String(opcao));
        executarOpcao(opcao);
        menu();
      }
      entradaSerial = "";
    } else {
      entradaSerial += c;
    }
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(BOTAO, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);
  pinMode(POT, INPUT);
  digitalWrite(LED_ONBOARD, LOW);

  analogReadResolution(9);

  blinkTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(blinkTimer, &toggleBlink, true);
  timerAlarmWrite(blinkTimer, TEMPO_BLINK_US, true);
  timerAlarmEnable(blinkTimer);

  timerWatchdog = timerBegin(1, 80, true);
  timerAttachInterrupt(timerWatchdog, &resetModule, true);
  timerAlarmWrite(timerWatchdog, wdtTimeout * 1000, false);
  timerAlarmEnable(timerWatchdog);

  attachInterrupt(digitalPinToInterrupt(BOTAO), botaoISR, CHANGE);

  Serial.println("Inicialização completa.");
  menu();
}

void loop() {
  timerWrite(timerWatchdog, 0);

  int leitura = analogRead(POT);
  int mapeado = map(leitura, 0, 1023, 0, 255);
  if (abs(mapeado - estado.ultimoPotMapeado) > 3) {
    estado.valorLedAzul = mapeado;
    estado.fonteUltimaAtualizacao = "potenciometro";
    log("Pot: bruto=" + String(leitura) + " mapeado=" + String(mapeado));
    estado.ultimoPotMapeado = mapeado;
  }

  analogWrite(LED_AZUL, estado.valorLedAzul);

  carga();
  processarEntradaSerial();

  delay(10);
}
