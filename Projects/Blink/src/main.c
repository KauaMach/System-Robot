#include <stdio.h>  // Biblioteca padrão de entrada e saída
#include "freertos/FreeRTOS.h"  // Biblioteca principal do FreeRTOS
#include "freertos/task.h"  // Biblioteca para criação e controle de tarefas
#include "driver/gpio.h"  // Biblioteca para controle de pinos GPIO
#include "esp_log.h"  // Biblioteca de log do ESP-IDF



#define BLINK_GPIO GPIO_NUM_2
#define LOG_STATUS  0
static const char *TAG = "BLINK";  // Tag usada nos logs

void app_main(void)
{
    man1();
}


void man1(void){
        // Reinicia a configuração do pino definido, garantindo que ele esteja em um estado conhecido
    gpio_reset_pin(BLINK_GPIO);
    // Define o pino como saída (output), permitindo controlar o LED
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

   
    while (1) {
       
        if (LOG_STATUS){printf("LED ON\n");}
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);

      
        if (LOG_STATUS){printf("LED ON\n");}
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void man2(void){
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Iniciando controle do LED no GPIO %d", BLINK_GPIO);

    while (1) {
        gpio_set_level(BLINK_GPIO, 1);
        if (LOG_STATUS){ ESP_LOGI(TAG, "LED ON");}
        vTaskDelay(500 / portTICK_PERIOD_MS);

        gpio_set_level(BLINK_GPIO, 0);
        if (LOG_STATUS){ ESP_LOGI(TAG, "LED OFF");};
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}



