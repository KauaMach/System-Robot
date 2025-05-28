#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

//----------------------------------------------------------------
// Definições de pinos
#define LED_BLINK GPIO_NUM_2
#define LED_BUTTON GPIO_NUM_26
#define BUTTON GPIO_NUM_4

#define LOG_STATUS 1
static const char *TAG = "Status";

// Protótipos
void init_gpio(void);
void blink(void);
void button(void);
void carga(int quantidade);
void teste(void);

//-------------------------------------------------------------------
// Variáveis estáticas para controle de tempo e estado
static TickType_t last_blink_time = 0; // Tempo do último piscar
static bool blink_led_state = false; // Estado do LED de piscar

static TickType_t last_button_time = 0; // Tempo do último botão pressionado
static bool button_led_state = false; // Estado do LED do botão
//-------------------------------------------------------------------

void app_main(void) {
    init_gpio();

    while (1) {
        blink();
        button();
        //carga(4000); // Chama a função carga com 5 como argumento
        //teste();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void init_gpio(void) { // Inicializa os pinos
    gpio_reset_pin(LED_BLINK);
    gpio_reset_pin(LED_BUTTON);
    gpio_reset_pin(BUTTON);

    gpio_set_direction(LED_BLINK, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BUTTON, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
}

void blink(void) { // Pisca o LED a cada 100ms
    TickType_t now = xTaskGetTickCount(); // Tempo atual
    // Verifica se o tempo atual é maior ou igual ao tempo do último piscar + 100ms
    // Se sim, inverte o estado do LED e atualiza o tempo do último piscar
    // Se não, não faz nada

    if ((now - last_blink_time) >= pdMS_TO_TICKS(100)) { // Verifica o estado do LED
        // Inverte o estado do LED
        // Se o LED estiver desligado, liga o LED e atualiza o tempo do último piscar
        // Se o LED estiver ligado, desliga o LED e atualiza o tempo do último piscar
        blink_led_state = !blink_led_state;
        gpio_set_level(LED_BLINK, blink_led_state);

        if (LOG_STATUS) {
            ESP_LOGI(TAG, "LED Blink: %s", blink_led_state ? "ON" : "OFF");
        }

        last_blink_time = now;
    }
}

void button(void) {// Verifica o estado do botão e acende o LED correspondente
    TickType_t now = xTaskGetTickCount(); // Tempo atual
    // Verifica se o tempo atual é maior ou igual ao tempo do último botão pressionado + 50ms
    // Se sim, verifica o estado do botão
    // Se o botão estiver pressionado, inverte o estado do LED e atualiza o tempo do último botão pressionado


    if ((now - last_button_time) >= pdMS_TO_TICKS(50)) { // Verifica o estado do botão
        // Lê o estado do botão (1 = pressionado, 0 = solto)
        // Se o botão estiver pressionado, inverte o estado do LED e atualiza o tempo do último botão pressionado
        // Se o botão estiver solto, apaga o LED e atualiza o tempo do último botão pressionado
        int button_state = gpio_get_level(BUTTON);

        if (button_state == 1) {
            button_led_state = !button_led_state;
            gpio_set_level(LED_BUTTON, button_led_state);

            if (LOG_STATUS) {
                ESP_LOGI(TAG, "Botão pressionado - LED: %s", button_led_state ? "LIGADO" : "DESLIGADO");
            }
        } else {
            gpio_set_level(LED_BUTTON, 0);

            if (LOG_STATUS) {
                ESP_LOGI(TAG, "Botão solto - LED: DESLIGADO");
            }
        }

        last_button_time = now;
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

void teste(void) {
    int button_state = gpio_get_level(BUTTON);
    ESP_LOGI(TAG, "state: %d", button_state);
   
    
    
    vTaskDelay(10 / portTICK_PERIOD_MS); 
}