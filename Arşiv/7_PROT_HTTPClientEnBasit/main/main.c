#include <stdio.h>
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi_network.h"
/*
LED_ON(gpio) gpio_set_level(gpio, 1)
LED_OFF(gpio) gpio_set_level(gpio, 0)
LED_TOGGLE(gpio) gpio_set_level(gpio, !gpio_get_level(gpio))
BUTTON1_READ() gpio_get_level(BUTTON_1_GPIO)
BUTTON2_READ() gpio_get_level(BUTTON_2_GPIO)
*/
EventGroupHandle_t wifi_event_group; 


void app_main(void)
{
    LED_INIT();
    LED_ALL_ON();
    vTaskDelay(pdMS_TO_TICKS(100));
    LED_ALL_OFF();
    wifi_event_group = xEventGroupCreate();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_network_init(wifi_event_group);
    ESP_LOGW("TEST", "STA MODE");
    wifi_network_station_mode();
     
    while(1)
    {
        // Burada olay başarılı bir şekilde okunmakta!
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}