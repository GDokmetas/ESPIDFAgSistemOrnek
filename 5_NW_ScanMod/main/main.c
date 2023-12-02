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


// Scan ile ilgili fonksiyonlar

/*
Tarama yapması için STA modda olması gerekli
* esp_wifi_scan_start(const wifi_scan_config_t *config, bool block)
* esp_wifi_scan_stop(void)
* esp_wifi_scan_get_ap_num(uint16_t *number) -> son taramada kaç adet AP bulunduğunu al
* esp_wifi_scan_get_ap_records(uint16_t *number, wifi_ap_record_t *ap_records) -> Son taramadaki AP kayıtlarını al (aldıktan sonra siler)
* esp_wifi_clear_ap_list(void) -> Son taramadaki AP listesini siler
*  
*/


static esp_netif_t *sta_netif;

static void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    sta_netif = esp_netif_create_default_wifi_sta();
    //assert(sta_netif) kullanılabilir
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
}

static void wifi_scan(void)
{
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    // Hepsini taraması için boş bırakıyoruz
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {
                .min = 100,
                .max = 500
            },
        }
    };

    esp_wifi_scan_start(&scan_config, true); // Blocking -> true Blocking olmazsa event beklenmeli!
}

void app_main(void)
{
    LED_INIT();
    LED_ALL_ON();
    vTaskDelay(pdMS_TO_TICKS(100));
    LED_ALL_OFF();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();
    wifi_scan();
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    ESP_LOGI("WIFI", "AP Count: %d", ap_count);
    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    esp_wifi_scan_get_ap_records(&ap_count, ap_records);
    for(int i = 0; i < ap_count; i++)
    {
        ESP_LOGI("WIFI", "SSID: %s", ap_records[i].ssid);
        ESP_LOGI("WIFI", "RSSI: %d", ap_records[i].rssi);
        ESP_LOGI("WIFI", "Channel: %d", ap_records[i].primary);
        ESP_LOGI("WIFI", "MAC: %02X:%02X:%02X:%02X:%02X:%02X", ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2], ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
        ESP_LOGI("WIFI", "Auth Mode: %d", ap_records[i].authmode);
        ESP_LOGI("WIFI", "Pairwise Cipher: %d", ap_records[i].pairwise_cipher);
        ESP_LOGI("WIFI", "Group Cipher: %d", ap_records[i].group_cipher);
        ESP_LOGI("WIFI", "Country CC: %s", ap_records[i].country.cc);
        ESP_LOGI("WIFI", "Country SCHAN: %d", ap_records[i].country.schan);
        ESP_LOGI("WIFI", "Country NCHAN: %d", ap_records[i].country.nchan);
        ESP_LOGI("WIFI", "Country MAX_TX_PWR: %d", ap_records[i].country.max_tx_power);
        ESP_LOGI("WIFI", "Country POLICY: %d", ap_records[i].country.policy);
    }

    free(ap_records);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}