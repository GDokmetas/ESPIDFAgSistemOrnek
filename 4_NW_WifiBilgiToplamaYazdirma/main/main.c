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

static esp_netif_t *wifi_netif;
static esp_netif_t *ap_netif;

void wifi_init_generic(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_netif.html
    // AP + STA modda iki arayüz de oluşturulmalı
    wifi_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();
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

    if (strlen(WIFI_AP_PASS) == 0)
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


//! Tasks 

static void log_task(void *arg)
{
    while(1)
    {
        wifi_mode_t wifi_mode;
        esp_wifi_get_mode(&wifi_mode);
        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);
    
        //uint8_t bssid[6];                     // MAC address of AP 
        // *uint8_t ssid[33];                   // SSID of AP 
        //uint8_t primary;                      // channel of AP 
        //wifi_second_chan_t second;            // secondary channel of AP 
        //int8_t  rssi;                         // signal strength of AP 
        //wifi_auth_mode_t authmode;            // authmode of AP 

        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        int rssi;
        esp_wifi_sta_get_rssi(&rssi); 
        esp_netif_ip_info_t ip;
        esp_netif_get_ip_info(wifi_netif, &ip);
        char * hostname = NULL;
        esp_netif_get_hostname(wifi_netif, &hostname);
        esp_netif_dhcp_status_t dhcp_status;
        esp_netif_dhcpc_get_status(wifi_netif, &dhcp_status);
        esp_netif_dns_info_t dns_info;
        esp_netif_get_dns_info(wifi_netif, ESP_NETIF_DNS_MAIN, &dns_info);

        /* Örnek çıktı
        I (54266) WIFI: AP SSID: MonsterHotspot
        I (54266) WIFI: AP RSSI: -54
        I (54266) WIFI: AP Authmode: 3
        I (54266) WIFI: AP Channel: 13
        I (54266) WIFI: AP MAC: 06:ec:d8:24:ce:94
        I (54276) WIFI: STA RSSI: -54
        I (54276) WIFI: STA MAC: e0:5a:1b:6c:ae:28
        I (54286) WIFI: STA IP: 192.168.137.142
        I (54286) WIFI: STA Netmask: 255.255.255.0
        I (54296) WIFI: STA Gateway: 192.168.137.1
        I (54296) WIFI: STA Hostname: espressif
        I (54306) WIFI: STA DHCP Status: 1
        I (54306) WIFI: STA DNS IP: 192.168.137.1
        */

        ESP_LOGI("WIFI", "Wifi Mode: %d", wifi_mode);
        ESP_LOGI("WIFI", "AP SSID: %s", ap_info.ssid);
        ESP_LOGI("WIFI", "AP RSSI: %d", ap_info.rssi);
        ESP_LOGI("WIFI", "AP Authmode: %d", ap_info.authmode);
        ESP_LOGI("WIFI", "AP Channel: %d", ap_info.primary);
        ESP_LOGI("WIFI", "AP MAC: %02x:%02x:%02x:%02x:%02x:%02x", ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2], ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);
        ESP_LOGI("WIFI", "STA RSSI: %d", rssi);
        ESP_LOGI("WIFI", "STA MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI("WIFI", "STA IP: " IPSTR, IP2STR(&ip.ip));
        ESP_LOGI("WIFI", "STA Netmask: " IPSTR, IP2STR(&ip.netmask));
        ESP_LOGI("WIFI", "STA Gateway: " IPSTR, IP2STR(&ip.gw));
        ESP_LOGI("WIFI", "STA Hostname: %s", hostname);
        ESP_LOGI("WIFI", "STA DHCP Status: %d", dhcp_status);
        ESP_LOGI("WIFI", "STA DNS IP: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    vTaskDelete(NULL);
}


static bool firstTime = true;

static void button_task(void *arg)
{
    while(1)
    {
        if(!BUTTON1_READ())
        {
            wifi_go_offline();
            wifi_init_sta();
            ESP_LOGI("WIFI", "wifi_init_sta.");
            vTaskDelay(pdMS_TO_TICKS(100));
            if(firstTime)
            {
                firstTime = false;
                xTaskCreate(log_task, "log_task", 4048, NULL, 1, NULL);
            }
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

    vTaskDelete(NULL);

}

// Kullanılabilecek LOG-
/*
* Arayüzde AP seçme ve şifre girme işi yapılabilir mi????
* esp_wifi_get_mode(wifi_mode_t *mode) -> Wifi modu
* esp_wifi_sta_get_ap_info(wifi_ap_record_t *ap_info) -> STA modda bağlı AP bilgisini al
* esp_wifi_get_mac(wifi_interface_t ifx, uint8_t mac[6]) -> MAC adresi al
*esp_wifi_sta_get_rssi(int *rssi) -> Bağlı AP'nin RSSI bilgisi

*esp_netif_ip_info_t ip;
    if(esp_netif_get_ip_info(get_esp_interface_netif(ESP_IF_WIFI_STA), &ip) != ESP_OK){
    	log_e("Netif Get IP Failed!");
    }

    IP Adres bilgisi
*esp_netif_get_hostname() hostname 
*esp_netif_dhcpc_get_status() DHCP status
*esp_netif_get_dns_info() DNS bilgisini al
*
*
*/




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
    // Burada düğme okuyan ve wifi durumunu yazdıran iki ayrı task açılacak. 
    xTaskCreate(button_task, "button_task", 4048, NULL, 1, NULL);
    while(1)
    {
        vTaskDelay(100);
    }
}