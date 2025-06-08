//------------------------
// Definição de pinos
#define POT_PIN 4       // Entrada analógica
#define PWM_LED 32      // LED com PWM
#define BUTTON_PIN 15   // Botão com pull-up interno
#define DIGITAL_LED 26  // LED digital
#define ONBOARD_LED 2   // LED da placa

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
// Variávei PWM
int brilhoPWM = 0;

// Variávei LOG
bool statusLog = false;

//------------------------
// Funções
void brilhoLedPot();
void ledBotao();
void blink();
void carga(int qtn);
void processarEntradaSerial();
void menu();
void log(String msg);

//------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  pinMode(PWM_LED, OUTPUT);
  pinMode(DIGITAL_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ONBOARD_LED, OUTPUT);
}

//------------------------

void loop() {
  brilhoLedPot();
  ledBotao();
  blink();
  carga(quantidade);
  processarEntradaSerial();

  delay(42);
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
  if (!Serial.available()) return;

  String entrada = Serial.readStringUntil('\n');
  entrada.trim();

  if (entrada.equalsIgnoreCase("MENU")) {
    menu();
  }
}

//------------------------
// Função: carga
void carga(int qtn) {
  if (qtn <= 0) return;

  if (!cargaAtiva) {
    cargaAtiva = true;
    contadorCarga = 0;
    ultimoTempoCarga = millis();
  }

  if (cargaAtiva && millis() - ultimoTempoCarga >= 10) {
    log("CARGA");
    contadorCarga++;
    ultimoTempoCarga = millis();

    if (contadorCarga >= qtn) {
      cargaAtiva = false;
      quantidade = 0;
    }
  }
}

//------------------------
// Função: Menu interativo
void menu() {
  Serial.println("====== MENU ======");
  Serial.println("1 - Ligar LED Digital");
  Serial.println("2 - Desligar LED Digital");
  Serial.println("3 - Ajustar brilho PWM (0-255)");
  Serial.println("4 - Ajustar quantidade da função carga");
  Serial.println("5 - Ativar log");
  Serial.println("6 - Desativar log");
  Serial.println("==================");
  Serial.println("Digite a opção desejada: ");

  while (!Serial.available()) {
    // Aguarda entrada do usuário
  }

  String opcao = Serial.readStringUntil('\n');
  opcao.trim();

  if (opcao == "1") {
    digitalWrite(DIGITAL_LED, HIGH);
    Serial.println("LED Digital LIGADO");

  } else if (opcao == "2") {
    digitalWrite(DIGITAL_LED, LOW);
    Serial.println("LED Digital DESLIGADO");

  } else if (opcao == "3") {
    Serial.println("Digite o valor do brilho (0-255): ");
    while (!Serial.available()) {
      // Aguarda entrada
    }
    String valor = Serial.readStringUntil('\n');
    valor.trim();
    int brilho = constrain(valor.toInt(), 0, 255);
    brilhoPWM = brilho;
    analogWrite(PWM_LED, brilhoPWM);
    Serial.print("Brilho PWM ajustado para: ");
    Serial.println(brilhoPWM);

  } else if (opcao == "4") {
    Serial.println("Digite a quantidade para a função carga: ");
    while (!Serial.available()) {
      // Aguarda entrada
    }
    String valor = Serial.readStringUntil('\n');
    valor.trim();
    if (valor.toInt() > 0) {
      quantidade = valor.toInt();
    }

    Serial.print("Quantidade da função carga ajustada para: ");
    Serial.println(quantidade);

  } else if (opcao == "5") {
    statusLog = true;
    Serial.println("Log ATIVADO");

  } else if (opcao == "6") {
    statusLog = false;
    Serial.println("Log DESATIVADO");

  } else {
    Serial.println("Opção inválida. Tente novamente.");
  }
}

//------------------------
// Função: Log condicional
void log(String msg) {
  if (!statusLog) return;
  Serial.println(msg);
}
