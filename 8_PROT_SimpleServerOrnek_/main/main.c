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
#include "wn_http_server.h"
#include "wn_nvs.h"
//* Notlar
/*
esp_http_client_perform() varsayılan olarak blocking bir fonksiyondur. non-blocking için is_async üyesini 
true yapmak gereklidir. non-blocking modda bu perform fonksiyonunu birkaç kez çağırıp dönen veriyi kontrol
etmek gerekebilir. (!Şu an için non-blocking sadece HTTPS için geçerli)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
*/

//*Header Dosyasına Yazılacak kısım

#define WIFI_APP_LOG_ENABLED 1


//*.c dosyasına yazılacak kısım 
EventGroupHandle_t wifi_event_group; 
EventGroupHandle_t wn_server_event_group;


//* Method Handlers 

//* URIs



//* Error Handler



//* Event Handler 


//* Server functions 

void app_main(void)
{
    LED_INIT();
    LED_ALL_ON();
    vTaskDelay(pdMS_TO_TICKS(100));
    LED_ALL_OFF();
    wifi_event_group = xEventGroupCreate();
    wn_server_event_group = xEventGroupCreate();
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_network_init(wifi_event_group);
    wn_server_init(wn_server_event_group);
    ESP_LOGW("TEST", "AP MODE");
    wifi_network_softap_mode();
    // Burada default_event_loop wifi_network içerisinde oluşturuldu (server eventi için event loop'u kullanmak gerekecek)

    // İstenirse eventlar doğrudan handlerlara bağlanıp eventlarla yönlendirilebilen bir sunucu işlemi yapılabilir
    /*
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    */

   // Bizim kullanacağımız eventlar -> (İstemci bağlandı) IP_EVENT_STA_GOT_IP (İstemci Çıktı) IP_EVENT_STA_DISCONNECTED 
   #if (NVS_LOG_PARAMS_ENABLED)
    char nvs_buf[32];
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_AP_NAME_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "AP NAME: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_AP_PASS_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "AP PASS: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_STA_SSID_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "STA SSID: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_STA_PASS_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "STA PASS: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_MQTT_SERVER_NAME_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "MQTT SERVER NAME: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_MQTT_SERVER_PORT_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "MQTT SERVER PORT: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_MQTT_USER_NAME_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "MQTT USER NAME: %s", nvs_buf);
    memset(nvs_buf, 0, sizeof(nvs_buf));
    wn_nvs_get_str(WN_SERVER_NVS_MQTT_USER_PASS_KEY, nvs_buf, sizeof(nvs_buf));
    ESP_LOGI("TEST", "MQTT USER PASS: %s", nvs_buf);
    #endif
    
    while(1)
    {
        
        EventBits_t wifi_network_event_bits = xEventGroupGetBits(wifi_event_group);

        if(wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_SCAN_DONE)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_SCAN_DONE);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_SCAN_DONE");
            #endif   
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_START)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_START);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_START");
            #endif
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_STOP)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_STOP);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_STOP");
            #endif
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_CONNECTED)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_CONNECTED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_CONNECTED");
            #endif

        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_DISCONNECTED)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_DISCONNECTED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_DISCONNECTED");
            #endif
            //http_server_sta_disconnected_handler(); TODO: Event set yapılacak, kütüphanenin kendi loopu olacak.
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_AUTHMODE_CHANGE)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_AUTHMODE_CHANGE);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_AUTHMODE_CHANGE");
            #endif
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_AP_START)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_AP_START);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_AP_START");
            #endif

        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_AP_STOP)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_AP_STOP);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_AP_STOP");
            #endif
            wn_server_stop();
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_AP_STACONNECTED)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_AP_STACONNECTED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_AP_STACONNECTED");
            #endif
            wn_server_start();
            // Burada captive portal göstermeli 

        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_AP_STADISCONNECTED)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_AP_STADISCONNECTED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_AP_STADISCONNECTED");
            #endif
            wn_server_sta_disconnected_handler();
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_AP_PROBEREQRECVED)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_AP_PROBEREQRECVED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_AP_PROBEREQRECVED");
            #endif
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_BSS_RSSI_LOW)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_BSS_RSSI_LOW);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_BSS_RSSI_LOW");
            #endif
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_BEACON_TIMEOUT)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_BEACON_TIMEOUT);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_BEACON_TIMEOUT");
            #endif

        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_GOT_IP)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_GOT_IP);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_GOT_IP");
            #endif

        }       

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_STA_LOST_IP)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_STA_LOST_IP);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_STA_LOST_IP");
            #endif
        }

        else if (wifi_network_event_bits & WIFI_NETWORK_EVENT_BIT_AP_STAIPASSIGNED)
        {
            xEventGroupClearBits(wifi_event_group, WIFI_NETWORK_EVENT_BIT_AP_STAIPASSIGNED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "Got Event: WIFI_NETWORK_EVENT_BIT_AP_STAIPASSIGNED");
            #endif
            wn_server_sta_connected_handler();
        }

        else if (wifi_network_event_bits & ~(WIFI_NETWORK_INTERNAL_EVENT_MASK))
        {
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGW("HTTP SERVER", "UNKNOWN EVENT FROM WIFI_NETWORK");
            #endif
        }

        EventBits_t wn_server_event_bits = xEventGroupGetBits(wn_server_event_group);

        if (wn_server_event_bits & WN_SERVER_EVENT_ERROR)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_ERROR);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_ERROR");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_START)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_START);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_START");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_ON_CONNECTED)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_ON_CONNECTED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_ON_CONNECTED");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_ON_HEADER)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_ON_HEADER);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_ON_HEADER");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_HEADERS_SENT)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_HEADERS_SENT);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_HEADERS_SENT");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_ON_DATA)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_ON_DATA);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_ON_DATA");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_SENT_DATA)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_SENT_DATA);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_SENT_DATA");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_DISCONNECTED)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_DISCONNECTED);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_DISCONNECTED");
            #endif
        }
        else if (wn_server_event_bits & WN_SERVER_EVENT_STOP)
        {
            xEventGroupClearBits(wn_server_event_group, WN_SERVER_EVENT_STOP);
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGI("WIFI APP", "WN_SERVER Got Event: WN_SERVER_EVENT_STOP");
            #endif
        }
        else if (wn_server_event_bits & ~(WN_SERVER_INTERNAL_EVENT_MASK))
        {
            #if (WIFI_APP_LOG_ENABLED)
            ESP_LOGW("WIFI APP", "UNKNOWN EVENT FROM WN_SERVER");
            #endif
            
        }

                // Burada olay başarılı bir şekilde okunmakta!
        vTaskDelay(pdMS_TO_TICKS(10));

    } // while(1)    
} // app_main