#include <stdio.h>
#include "led.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// Bunlar değişebilmeli, değişken olarak mevcut olacak. 
#define WIFI_SSID "MonsterHotspot" // Wifi İsmi
#define WIFI_PASS "12345678" // Wifi şifresi 8 karakterden az olamaz.
#define WIFI_AP_SSID "MonsterAP" // AP İsmi
#define WIFI_AP_PASS "12345678" // AP Şifresi 8 karakterden az olamaz.
#define WIFI_CHANNEL 1 // Wifi kanalı 1-13 arasında olmalıdır.
#define MAX_STA_CONN 4 // En fazla kaç istemci bağlanabilir?

// Event handler AP + STA

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_WIFI_READY:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_WIFI_READY");

            }
            break;

            case WIFI_EVENT_STA_START:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_START");
                esp_wifi_connect();
            }
            break;

            case WIFI_EVENT_STA_STOP:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_STOP");

            }
            break;

            case WIFI_EVENT_STA_CONNECTED:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_CONNECTED");

            }
            break;

            case WIFI_EVENT_STA_DISCONNECTED:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_STA_DISCONNECTED");
                esp_wifi_connect();
                ESP_LOGI("WIFI", "Retry to connect to AP");
            }
            break;

            case WIFI_EVENT_AP_START:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_AP_START");

            }
            break;

            case WIFI_EVENT_AP_STOP:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_AP_STOP");

            }
            break;

            case WIFI_EVENT_AP_STACONNECTED:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_AP_STACONNECTED");

            }
            break;

            case WIFI_EVENT_AP_STADISCONNECTED:
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_AP_STADISCONNECTED");

            }
            break;
        }
    }
    else if(event_base == IP_EVENT)
    {
        switch(event_id)
        {
            // Bu kısım genişletilecek
            case IP_EVENT_STA_GOT_IP:
            {
                LED_ON(LED_GREEN);
                ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI("WIFI", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            }
            break;
        }
    }
}

void wifi_init_generic(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_netif.html
    // AP + STA modda iki arayüz de oluşturulmalı
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
}

void wifi_init_sta(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

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

    // Burada diğer işlemin gerçekleştirilmesini beklemek için xEventGroupWaitBits kullanılabilir.
}

void wifi_init_ap(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = true
            },
        },
    };

    if (strlen(WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI("WIFI", "wifi_init_softap finished. SSID:%s password:%s channel:%d", WIFI_AP_SSID, WIFI_AP_PASS, WIFI_CHANNEL);
}

void wifi_go_offline(void)
{
    // Burada wifi ayarları da sıfırlanacak
    esp_wifi_disconnect();
    esp_wifi_restore();
    esp_wifi_stop();
}


void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    LED_INIT();
    LED_ALL_ON();
    vTaskDelay(pdMS_TO_TICKS(100));
    LED_ALL_OFF();
    wifi_init_generic();

    while(1)
    {
        if(!BUTTON1_READ())
        {
            wifi_go_offline();
            wifi_init_sta();
            ESP_LOGI("WIFI", "wifi_init_sta.");
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if(!BUTTON2_READ())
        {
            wifi_go_offline();
            wifi_init_ap();
            ESP_LOGI("WIFI", "wifi_init_ap.");
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay((TickType_t)1);
    }
}