
// Definição de pinos
#define ONBOARD_LED 2  // LED da placa
#define POT_PIN 4   // Entrada analógica
#define PWM_LED 32  // LED com PWM
#define BUTTON_PIN 15   // Botão com pull-up interno
#define DIGITAL_LED 26  // LED digital

//------------------------
// Variáveis debounce
bool estadoBotaoAnterior = HIGH;
bool estadoBotaoEstavel = HIGH;
unsigned long tempoUltimaMudanca = 0;
const unsigned long debounceDelay = 50;

//------------------------
// Variáveis para função carga
static int contadorCarga = 0;
static bool cargaAtiva = false;
static unsigned long ultimoTempoCarga = 0;
int quantidade = 0;

//------------------------
// Variáveis Globais auxiliares

int brilhoPWM = 0;
bool statusLog = false;  // Variávei LOG
bool menuAtivo = false;
int estadoMenu = 0;
String entradaSerial = "";


//------------------------
// Funções
void menu();
void brilhoLedPot();
void log(String msg);
void processarEntradaSerial();



//------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  pinMode(PWM_LED, OUTPUT);
  pinMode(DIGITAL_LED, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ledBotao, HIGH);
}

//------------------------

void loop() {
  brilhoLedPot();
  processarEntradaSerial();
}

//------------------------
// Função: Pisca o LED onboard sem bloquear
void blink() {
  digitalWrite(ONBOARD_LED, (millis() / 1000) % 2);
}

//------------------------
// Função: Debounce e controle LED
void ledBotao() {
  bool leituraAtual = digitalRead(BUTTON_PIN);

  if (leituraAtual != estadoBotaoAnterior) {
    tempoUltimaMudanca = millis();
  }

  if ((millis() - tempoUltimaMudanca) > debounceDelay) {
    if (leituraAtual != estadoBotaoEstavel) {
      estadoBotaoEstavel = leituraAtual;
      digitalWrite(DIGITAL_LED, estadoBotaoEstavel);

      log(estadoBotaoEstavel ? "Botão solto - LED desligado" : "Botão pressionado - LED ligado");
    }
  }

  estadoBotaoAnterior = leituraAtual;
}

//------------------------
// Função: Leitura analógica e controle PWM
void brilhoLedPot() {
  static int ultimoValor = 0;
  int valorPOT = analogRead(POT_PIN);

  if (abs(valorPOT - ultimoValor) > 10) {
    brilhoPWM = map(valorPOT, 0, 4095, 0, 255);
    analogWrite(PWM_LED, brilhoPWM);

    log("Potenciômetro: " + String(valorPOT) + " | PWM: " + String(brilhoPWM));
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
// Função: carga
void carga() {
  if (quantidade <= 0) return;

  for (int i = 0; i < quantidade; i++) {
    // Faz uma tarefa pesada — aqui só imprime mas poderia ser cálculo
    Serial.println("CARGA");
  }

  // Após o for, zera a carga para não rodar de novo
  quantidade = 0;
}


//------------------------
// Função: Menu interativo
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
      {
        int brilho = constrain(comando.toInt(), 0, 255);
        brilhoPWM = brilho;
        analogWrite(PWM_LED, brilhoPWM);
        Serial.print("Brilho PWM ajustado para: ");
        Serial.println(brilhoPWM);
        menuAtivo = false;
      }
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

String esperarSerial() {
  if (Serial.available()) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();
    return entrada;
  }
  return "";
}
//------------------------

// Função: Log condicional
void log(String msg) {
  if (!statusLog) return;
  Serial.println(msg);
}
















// Definição de pinos
#define BUTTON_PIN 15   // Botão com pull-up interno
#define DIGITAL_LED 26  // LED digital

//------------------------
// Variáveis debounce
bool estadoBotaoAnterior = HIGH;
bool estadoBotaoEstavel = HIGH;
unsigned long tempoUltimaMudanca = 0;
const unsigned long debounceDelay = 50;


//------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  pinMode(DIGITAL_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ledBotao, HIGH);
}

void loop() {
}

void ledBotao() {
  bool leituraAtual = digitalRead(BUTTON_PIN);
  if (leituraAtual != estadoBotaoAnterior) {
    tempoUltimaMudanca = millis();
  }
  if ((millis() - tempoUltimaMudanca) > debounceDelay) {
    if (leituraAtual != estadoBotaoEstavel) {
      estadoBotaoEstavel = leituraAtual;
      digitalWrite(DIGITAL_LED, estadoBotaoEstavel);
       Serial.printf(estadoBotaoEstavel ? "Botão solto - LED desligado" : "Botão pressionado - LED ligado");
    }
  }
  estadoBotaoAnterior = leituraAtual;
}








