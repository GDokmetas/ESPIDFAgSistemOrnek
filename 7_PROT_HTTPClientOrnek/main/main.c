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
#include "esp_http_client.h"
//* Notlar
/*
esp_http_client_perform() varsayılan olarak blocking bir fonksiyondur. non-blocking için is_async üyesini 
true yapmak gereklidir. non-blocking modda bu perform fonksiyonunu birkaç kez çağırıp dönen veriyi kontrol
etmek gerekebilir. (!Şu an için non-blocking sadece HTTPS için geçerli)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))


,*/



//*Header Dosyasına Yazılacak kısım
#define HTTP_LIB_LOG_ENABLED 1



//*.c dosyasına yazılacak kısım 
EventGroupHandle_t wifi_event_group; 
//TODO: Bu kısımda bütün rest'e uygun veriyi doğru bir şekilde değerlendiren bir yapı düzenlenecektir.

/* Event içinde olan veriler
    esp_http_client_event_id_t event_id;    // event_id, to know the cause of the event 
    esp_http_client_handle_t client;        // esp_http_client_handle_t context 
    void *data;                             // data of the event 
    int data_len;                           // data length of data 
    void *user_data;                        // user_data context, from esp_http_client_config_t user_data 
    char *header_key;                       // For HTTP_EVENT_ON_HEADER event_id, it's store current http header key
    char *header_value;   
*/
esp_err_t http_event_handler(esp_http_client_event_t *evnt)
{
    static char *output_buffer;  // chunked response'da gelen verinin kaydedileceği kısım 
    static int output_len;       // Kaç baytlık veri buffer'a kaydedildi? 
    switch (evnt->event_id)
    {
        case HTTP_EVENT_ERROR:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_ERROR");
            #endif
        }
        break;

        case HTTP_EVENT_ON_CONNECTED:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_CONNECTED");
            #endif
        }
        break;

        case HTTP_EVENT_HEADERS_SENT:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_HEADERS_SENT");
            #endif
        }
        break;

        case HTTP_EVENT_ON_HEADER:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_HEADER");
            #endif
        }
        break;

        case HTTP_EVENT_ON_DATA:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_DATA");
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_DATA, len=%d", evnt->data_len);
            #endif
        }
        break;

        case HTTP_EVENT_ON_FINISH:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_ON_FINISH");
            #endif
        }
        break;

        case HTTP_EVENT_DISCONNECTED:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_DISCONNECTED");
            #endif
        }
        break;

        case HTTP_EVENT_REDIRECT:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGI("HTTP", "HTTP_EVENT_REDIRECT");
            #endif
        }
        break;

        default:
        {
            #if (HTTP_LIB_LOG_ENABLED)
            ESP_LOGW("HTTP", "HTTP_EVENT_UNKNOWN");
            #endif
        }
        break;
    }

    return ESP_OK;
}


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
    ESP_LOGW("TEST", "AP MODE");
    wifi_network_station_mode();
     
    while(1)
    {
        // Burada olay başarılı bir şekilde okunmakta!
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}