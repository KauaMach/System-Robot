#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define o pino onde o LED está conectado
#define BLINK_GPIO GPIO_NUM_4

// Variáveis para controle de tempo
unsigned long previousMillis = 0;  // Armazena o tempo do último piscar
const long interval = 500;          // Intervalo de 500 ms

extern "C" void app_main(void)
{
    // Reinicia o pino para garantir que ele esteja em um estado conhecido
    gpio_reset_pin(BLINK_GPIO);
    // Configura o pino como saída
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    

    // Loop infinito
    while (1) {
        // Obtém o tempo atual em ticks (milissegundos)
        unsigned long currentMillis = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // Verifica se o intervalo de tempo foi alcançado
        if (currentMillis - previousMillis >= interval) {
            // Armazena o tempo atual para o próximo cálculo de intervalo
            previousMillis = currentMillis;

            // Liga ou desliga o LED com base no estado atual
            static bool ledState = false;
            ledState = !ledState;
            gpio_set_level(BLINK_GPIO, ledState ? 1 : 0);
            
        }
    }
}


