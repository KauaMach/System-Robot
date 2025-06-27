#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h> // Para lidar com URIs com variáveis (ex: /toggle/LED1)

#include <esp_timer.h>
#include "esp_system.h"
#include "rom/ets_sys.h"

// --- Configurações de Rede ---
#define WIFI_SSID "UFPI" // Altere para o nome da sua rede Wi-Fi
#define WIFI_PASSWORD "" // Altere para a senha da sua rede Wi-Fi
// Opcional: para se conectar a um Wi-Fi existente, descomente e configure
// #define WIFI_CHANNEL 6

// --- Servidor Web ---
WebServer server(80);

// --- Definição de pinos ---
#define ONBOARD_LED 2    // LED da placa (ainda usado pelo timer blink)
#define BUTTON_PIN 15    // Botão com pull-up interno
#define DIGITAL_LED 26   // LED digital (controlado pelo botão e web)
#define POT_PIN 4        // Entrada analógica
#define PWM_LED 32       // LED com PWM (controlado pelo potenciômetro e slider web)

// --- Variáveis Globais de Controle ---
volatile bool ledDigitalState = false; // Estado do LED digital (para o botão físico e web)
int quantidadeCarga = 0; // Controle da carga de CPU
int brilhoPWM = 0; // Valor do PWM para o LED (0-255)
bool statusLog = false; // Estado do log

// --- Variáveis para controle de tempo do botão (debounce) ---
volatile unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 50; // ms

// --- Função Blink ---
hw_timer_t *timerBlink = NULL; // Timer para blink
void IRAM_ATTR blinkISR() {
  digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED)); // inverte o estado do LED
}

// --- Função Botão ---
void IRAM_ATTR botaoISR() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastButtonPressTime > debounceDelay) {
    if (digitalRead(BUTTON_PIN) == LOW) { // Botão pressionado
      ledDigitalState = !ledDigitalState; // Inverte o estado do LED digital
      digitalWrite(DIGITAL_LED, ledDigitalState ? HIGH : LOW);
      // Você pode adicionar um evento para notificar o cliente web aqui, se necessário
    }
    lastButtonPressTime = currentMillis;
  }
}

// --- Função Watchdog ---
const int wdtTimeout = 3000;       // Timeout do watchdog em ms (3 segundos)
hw_timer_t *timerWatchdog = NULL;  // Ponteiro para timer hardware
void IRAM_ATTR resetModule() {
  Serial.println("\nReiniciando por watchdog!");
  esp_restart(); // para reiniciar o ESP.
}

// --- Funções Auxiliares ---
void sobrecarregar(int carga);
void brilhoLedPot();
void log(String msg);
void sendHtml(); // Função para enviar a página HTML para o cliente

// --- Setup ---
void setup() {
  Serial.begin(115200);
  delay(100);

  // --- Configuração de pinos ---
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(DIGITAL_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PWM_LED, OUTPUT);

  // --- Conexão Wi-Fi ---
  Serial.print("Conectando ao WiFi ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //, WIFI_CHANNEL); // Descomente o canal se estiver usando
  
  // Espera pela conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // --- Configuração do servidor web ---
  server.on("/", sendHtml); // Rota principal, envia a página HTML

  // Rota para controlar o LED Digital
  server.on(UriBraces("/toggleLedDigital/{}"), []() {
    String state = server.pathArg(0);
    if (state == "on") {
      ledDigitalState = true;
      digitalWrite(DIGITAL_LED, HIGH);
    } else if (state == "off") {
      ledDigitalState = false;
      digitalWrite(DIGITAL_LED, LOW);
    }
    log("LED Digital: " + String(ledDigitalState ? "LIGADO" : "DESLIGADO"));
    sendHtml(); // Atualiza a página após a ação
  });

  // Rota para ajustar o brilho do PWM
  server.on(UriBraces("/setBrilhoPWM/{}"), []() {
    String value = server.pathArg(0);
    brilhoPWM = constrain(value.toInt(), 0, 255);
    analogWrite(PWM_LED, brilhoPWM);
    log("Brilho PWM ajustado para: " + String(brilhoPWM));
    sendHtml(); // Atualiza a página após a ação
  });

  // Rota para ajustar a quantidade da função de carga
  server.on(UriBraces("/setCarga/{}"), []() {
    String value = server.pathArg(0);
    quantidadeCarga = value.toInt();
    if (quantidadeCarga < 0) quantidadeCarga = 0; // Garante que a carga não seja negativa
    log("Carga ajustada para: " + String(quantidadeCarga));
    sendHtml(); // Atualiza a página após a ação
  });

  // Rota para ativar/desativar o log
  server.on(UriBraces("/toggleLog/{}"), []() {
    String state = server.pathArg(0);
    if (state == "on") {
      statusLog = true;
    } else if (state == "off") {
      statusLog = false;
    }
    log("Log: " + String(statusLog ? "ATIVADO" : "DESATIVADO"));
    sendHtml(); // Atualiza a página após a ação
  });

  // Rota para obter o estado atual de todos os componentes
  server.on("/getState", []() {
    String jsonResponse = "{";
    jsonResponse += "\"ledDigital\": " + String(ledDigitalState ? "true" : "false") + ",";
    jsonResponse += "\"brilhoPWM\": " + String(brilhoPWM) + ",";
    jsonResponse += "\"quantidadeCarga\": " + String(quantidadeCarga) + ",";
    jsonResponse += "\"statusLog\": " + String(statusLog ? "true" : "false");
    jsonResponse += "}";
    server.send(200, "application/json", jsonResponse);
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");

  // --- Blink com timer ---
  timerBlink = timerBegin(0, 80, true); // 80MHz / 80 = 1MHz → 1 tick = 1 microsegundo
  timerAttachInterrupt(timerBlink, &blinkISR, true);
  timerAlarmWrite(timerBlink, 500000, true); // alarme para 500.000 us = 500 ms
  timerAlarmEnable(timerBlink);

  // --- Interrupção do botão ---
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), botaoISR, CHANGE);

  // --- Configura Watchdog ---
  timerWatchdog = timerBegin(1, 80, true); // 80 prescaler → 1 tick = 1 us
  timerAttachInterrupt(timerWatchdog, &resetModule, true);
  timerAlarmWrite(timerWatchdog, wdtTimeout * 1000, false); // timeout em micros
  timerAlarmEnable(timerWatchdog);
}

// --- Loop ---
void loop() {
  timerWrite(timerWatchdog, 0); // Alimenta o watchdog (reseta o timer)
  server.handleClient();       // Processa as requisições do cliente web
  brilhoLedPot();              // PWM com potenciômetro (controle físico)
  sobrecarregar(quantidadeCarga); // Função de carga
  delay(10); // Pequeno atraso para não sobrecarregar o loop
}

// --- Função: Carga simulada ---
void sobrecarregar(int carga) {
  volatile unsigned long i = 0;
  while (i < carga) {
    i++;
    // int c = pow(i, 2); // Removi a operação pow para evitar float e otimizar para inteiros
    yield(); // Permite que outras tarefas sejam executadas (importante para o servidor web)
  }
}

// --- Função: Leitura analógica e controle PWM ---
void brilhoLedPot() {
  static int ultimoValor = 0;
  int valorPOT = analogRead(POT_PIN);
  // Só atualiza se houver uma mudança significativa para evitar ruído
  if (abs(valorPOT - ultimoValor) > 20) { // Aumentei o limiar para 20 para reduzir a sensibilidade a ruído
    brilhoPWM = map(valorPOT, 0, 4095, 0, 255);
    analogWrite(PWM_LED, brilhoPWM);
    // Não enviamos log para o SerialBT aqui, mas podemos atualizar o cliente web via WebSockets (mais avançado) ou polling
    // Para simplificar, o log via web será disparado pelas ações da interface.
    ultimoValor = valorPOT;
  }
}

// --- Log condicional (agora para Serial e, se quiser, pode ser para o console do navegador) ---
void log(String msg) {
  if (statusLog) {
    Serial.print("LOG: ");
    Serial.println(msg);
    // Em um cenário mais avançado, você pode enviar este log para a página web via WebSockets.
  }
}

// --- Função para enviar a página HTML para o cliente ---
void sendHtml() {
  String response = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Controle ESP32</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial, sans-serif; text-align: center; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 500px; margin: 0 auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        .control-group { margin-bottom: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 5px; background-color: #f9f9f9; }
        .btn {
          background-color: #4CAF50; /* Verde padrão */
          border: none;
          color: white;
          padding: 10px 20px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 16px;
          margin: 4px 2px;
          cursor: pointer;
          border-radius: 5px;
          transition: background-color 0.3s ease;
        }
        .btn:hover { background-color: #45a049; }
        .btn.OFF { background-color: #f44336; } /* Vermelho para OFF */
        .btn.ON { background-color: #4CAF50; } /* Verde para ON */
        .slider-label { margin-top: 10px; font-weight: bold; }
        #potSlider { width: 80%; }
        #logText {
          width: 90%;
          height: 150px;
          margin-top: 10px;
          padding: 10px;
          border: 1px solid #ccc;
          border-radius: 5px;
          font-family: monospace;
          background-color: #eee;
          overflow-y: scroll;
          white-space: pre-wrap; /* Mantém quebras de linha */
        }
        input[type="number"] {
            width: 100px;
            padding: 8px;
            margin-left: 10px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Controle ESP32 via Web</h1>

        <div class="control-group">
          <h2>LED Digital</h2>
          <button id="ledDigitalBtn" class="btn">LED_DIGITAL_STATE</button>
        </div>

        <div class="control-group">
          <h2>Controle PWM do LED</h2>
          <label for="potSlider" class="slider-label">Brilho do LED (0-255): <span id="brilhoValue">0</span></label>
          <input type="range" id="potSlider" min="0" max="255" value="0">
        </div>

        <div class="control-group">
          <h2>Carga de CPU</h2>
          <label for="cargaInput">Quantidade da Carga:</label>
          <input type="number" id="cargaInput" value="0">
          <button id="setCargaBtn" class="btn">Aplicar Carga</button>
        </div>

        <div class="control-group">
          <h2>Log de Eventos</h2>
          <button id="toggleLogBtn" class="btn">LOG_STATE</button>
          <textarea id="logText" readonly></textarea>
        </div>
      </div>

      <script>
        const ledDigitalBtn = document.getElementById('ledDigitalBtn');
        const potSlider = document.getElementById('potSlider');
        const brilhoValue = document.getElementById('brilhoValue');
        const cargaInput = document.getElementById('cargaInput');
        const setCargaBtn = document.getElementById('setCargaBtn');
        const toggleLogBtn = document.getElementById('toggleLogBtn');
        const logText = document.getElementById('logText');

        // Função para obter o estado atual do ESP32
        async function fetchState() {
          try {
            const response = await fetch('/getState');
            const data = await response.json();
            
            // Atualiza o botão do LED Digital
            ledDigitalBtn.textContent = data.ledDigital ? "Ligar LED" : "Desligar LED";
            ledDigitalBtn.classList.remove('ON', 'OFF');
            ledDigitalBtn.classList.add(data.ledDigital ? 'ON' : 'OFF');

            // Atualiza o slider do PWM
            potSlider.value = data.brilhoPWM;
            brilhoValue.textContent = data.brilhoPWM;

            // Atualiza o campo da carga
            cargaInput.value = data.quantidadeCarga;

            // Atualiza o botão do Log
            toggleLogBtn.textContent = data.statusLog ? "Desligar Log" : "Ligar Log";
            toggleLogBtn.classList.remove('ON', 'OFF');
            toggleLogBtn.classList.add(data.statusLog ? 'ON' : 'OFF');

          } catch (error) {
            console.error("Erro ao buscar estado:", error);
          }
        }

        // --- Event Listeners ---

        // LED Digital
        ledDigitalBtn.addEventListener('click', async () => {
          const currentState = ledDigitalBtn.classList.contains('ON');
          const newState = currentState ? 'off' : 'on';
          await fetch(`/toggleLedDigital/${newState}`);
          await fetchState(); // Atualiza o estado após a ação
        });

        // Potenciômetro (Slider)
        potSlider.addEventListener('input', async () => {
          const value = potSlider.value;
          brilhoValue.textContent = value;
          // Envia o valor do slider para o ESP32 em tempo real
          await fetch(`/setBrilhoPWM/${value}`);
          // Não é necessário fetchState aqui, pois a atualização visual já ocorre
        });

        // Carga de CPU
        setCargaBtn.addEventListener('click', async () => {
          const value = cargaInput.value;
          await fetch(`/setCarga/${value}`);
          await fetchState();
        });

        // Log de Eventos
        toggleLogBtn.addEventListener('click', async () => {
          const currentState = toggleLogBtn.classList.contains('ON');
          const newState = currentState ? 'off' : 'on';
          await fetch(`/toggleLog/${newState}`);
          await fetchState();
        });
        
        // --- Função para simular o log na caixa de texto (apenas para demonstração) ---
        // Em uma aplicação real, você usaria WebSockets para logs em tempo real.
        let logCounter = 0;
        function appendLog(message) {
            const now = new Date();
            const timeString = `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}`;
            logText.value += `[${timeString}] ${message}\n`;
            logText.scrollTop = logText.scrollHeight; // Scroll para o final
            logCounter++;
            if (logCounter > 100) { // Limita o número de linhas para evitar sobrecarga
                const lines = logText.value.split('\n');
                logText.value = lines.slice(lines.length - 100).join('\n');
            }
        }

        // Inicializa o estado da interface ao carregar a página
        window.onload = () => {
            fetchState();
            // Simula alguns logs iniciais ou se você quiser, pode puxar de um endpoint
            appendLog("Bem-vindo ao controle ESP32!");
            appendLog("Use os botões e sliders para interagir.");
        };

        // Polling para manter o estado atualizado (pode ser substituído por WebSockets para melhor performance)
        setInterval(fetchState, 1000); // Atualiza o estado a cada 1 segundo
      </script>
    </body>
    </html>
  )";
  
  // As substituições abaixo são um fallback. O JS no cliente fará as atualizações dinamicamente.
  response.replace("LED_DIGITAL_STATE", ledDigitalState ? "Desligar LED" : "Ligar LED");
  response.replace("LOG_STATE", statusLog ? "Desligar Log" : "Ligar Log");

  server.send(200, "text/html", response);
}