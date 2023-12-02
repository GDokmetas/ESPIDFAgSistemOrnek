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
/*
LED_ON(gpio) gpio_set_level(gpio, 1)
LED_OFF(gpio) gpio_set_level(gpio, 0)
LED_TOGGLE(gpio) gpio_set_level(gpio, !gpio_get_level(gpio))
BUTTON1_READ() gpio_get_level(BUTTON_1_GPIO)
BUTTON2_READ() gpio_get_level(BUTTON_2_GPIO)
*/

/*
*İlk Örnekte Kullnanılan Kütüphaneler
*-> EventGroup
*-> NetIF
*-> ESP Event
*-> 
*/

#define WIFI_SSID "MonsterHotspot"
#define WIFI_PASS "12345678"
#define WIFI_MAX_RETRY 5


static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0 
#define WIFI_FAIL_BIT      BIT1 

static const char *TAG = "WIFI";
static int wifi_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (wifi_retry_num < WIFI_MAX_RETRY)
        {
            esp_wifi_connect();
            wifi_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        LED_ON(LED_GREEN);
        ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    esp_netif_init(); //! Gerçek yazılımda her zaman error check yap!
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);



    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
            .sae_h2e_identifier = ""
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}   

void app_main(void)
{
    LED_INIT();
    LED_ALL_ON();
    vTaskDelay(pdMS_TO_TICKS(100));
    LED_ALL_OFF();
    /*
    * Wifi için doc'da RF calibration başlığını okuyunuz
    Kalibrasyon verisi NVS'de saklanmaktadır. Eğer NVS yoksa full kalibrasyona girmektedir (100ms gecikmeli başlangıç)
    CONFIG_ESP_PHY_CALIBRATION_AND_DATA_STORAGE kısmına bakınız.  
    */
    /*
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    */
    wifi_init_sta();
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}