#include <stdio.h>               // Biblioteca padrão de entrada e saída
#include "freertos/FreeRTOS.h"   // Biblioteca principal do FreeRTOS
#include "freertos/task.h"       // Biblioteca para criação e controle de tarefas
#include "driver/gpio.h"         // Biblioteca para controle de pinos GPIO

#define PINO_LED GPIO_NUM_2      // Define o pino do LED (GPIO 2)
#define LOG_STATUS 1           // Define se o log será exibido (1 = sim, 0 = não)


void carga(int quantidade); 

void app_main(void)
{
    // Configura o pino do LED
    gpio_reset_pin(PINO_LED);                               // Reseta o pino para estado inicial
    gpio_set_direction(PINO_LED, GPIO_MODE_OUTPUT);         // Define o pino como saída

    TickType_t tempo_ultima_alteracao = xTaskGetTickCount(); // Armazena o tempo da última troca de estado
    static bool estado_led = false;                          // Estado inicial do LED: desligado

    while (1) {
        TickType_t tempo_atual = xTaskGetTickCount();

        if ((tempo_atual - tempo_ultima_alteracao) >= pdMS_TO_TICKS(500)) {
            estado_led = !estado_led;  // Alterna o estado do LED
            gpio_set_level(PINO_LED, estado_led ? 1 : 0); // Aplica o novo estado no pino

            if (LOG_STATUS) {
                printf("LED %s\n", estado_led ? "ON" : "OFF");
            }

            tempo_ultima_alteracao = tempo_atual; // Atualiza o tempo da última troca
        }
        carga(4000); // Chama a função carga com 5 como argumento

        vTaskDelay(50 / portTICK_PERIOD_MS);
    
    }
}

// Função que imprime "CARGA" um número específico de vezes
void carga(int quantidade)
{
    for (int i = 0; i < quantidade; i++) {
        printf("CARGA\n");
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); 

    
}
