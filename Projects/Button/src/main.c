#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_PIN GPIO_NUM_5
#define TAG "BOTAO_TESTE"

void app_main(void)
{
    // Configura o pino do botão como entrada
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    int last_state = 0;

    while (1) {
        int state = gpio_get_level(BUTTON_PIN);

        // Detecta transição de estado
        if (state != last_state) {
            if (state == 1) {
                ESP_LOGI(TAG, "Botão PRESSIONADO");
            } else {
                ESP_LOGI(TAG, "Botão SOLTO");
            }
            last_state = state;
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Delay para evitar bouncing e reduzir uso de CPU
    }
}
