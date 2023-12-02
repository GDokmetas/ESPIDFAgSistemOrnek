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

// TODO: STA -> Scan modları arasında geçişte sorun yaşanmakta. Bu yapıyı baştan kurmak ve dikkatle uygulamak gerekli. 
// TODO: Captive portal için ayrı bir dosya oluşturulacak.
// Scan command -> Scan wait -> scan done (Başta türlü olan scanlar kabul edilmeyecek, bağlandığı sırada scan iptal olacak)

// Scan ile ilgili fonksiyonlar
// TODO: Yapılacak işler 

/* 
- Wifi için task oluştur (AP, Scan, STA, Bağlanma işleri bu task üzerinden gerçekleştir.)
- Task içinde fonksiyonları çağır, bunların hiçbiri blocking olmasın.
- Task içinde bu fonksiyonların event döndürmesini bekle
- Fonksiyonlardan dönen değerleri denetleyerek işlemleri gerçekleştir.
- Kayıtlı WIFI ağ ayarları için NVS kullan. (Bu noktada bu veriler, kayıt edilirken elle girilecek.)

- Hemen bağlanma -
- Bir ağdan bağlantı koparsa hemen bağlanmayı dene, bağlanamazsa kısa offline'a geç ve tekrar dene.
- Kısa offline'dan sonra bağlanamazsa tarama moduna geçiş yap. 

-Tarama-

- Önce mevcut ağları tara, eğer mevcut ağ sayısı 0 ise Offline moda uzun süreli geç
- Mevcut ağ sayısı 0'dan büyükse kayıtlı ağ listesinde yer alan ağları tespit et (SSID'leri sırayla karşılaştır)
- Bulunan ağların RSSI değerlerini karşılaştır, en yüksek olanı seç ve bağlan
- Bağlantı başarılı olursa tarama verilerini temizle. Başarısız olursa RSSI değeri bir düşük olana bağlan.
- Bütün ağlara bağlantı başarısız olursa uzun offline'a geç. 

-> TODO: Bununla ilgili stateler çıkarılacak. 
-> TODO: Zamanlama için FreeRTOS software timer kullanılacak
-> Olaylar için ESP Event kütüphanesi ve RTOS eventler kullanılacak
-> İşlemler tek task içinde gerçekleşecek

*/
/*
Tarama yapması için STA modda olması gerekli
* esp_wifi_scan_start(const wifi_scan_config_t *config, bool block)
* esp_wifi_scan_stop(void)
* esp_wifi_scan_get_ap_num(uint16_t *number) -> son taramada kaç adet AP bulunduğunu al
* esp_wifi_scan_get_ap_records(uint16_t *number, wifi_ap_record_t *ap_records) -> Son taramadaki AP kayıtlarını al (aldıktan sonra siler)
* esp_wifi_clear_ap_list(void) -> Son taramadaki AP listesini siler
*  
*/

// Defines

#define WIFI_NETWORKS_MAX 10
#define WIFI_AP_SSID "MonsterAP" // AP İsmi
#define WIFI_AP_PASS "12345678" // AP Şifresi 8 karakterden az olamaz.
#define WIFI_CHANNEL 1 // Wifi kanalı 1-13 arasında olmalıdır.
#define MAX_STA_CONN 4 // En fazla kaç istemci bağlanabilir?


// Typedefs 
typedef enum {
    WIFI_NETWORK_NOT_VALID = 0,
    WIFI_NETWORK_VALID = 1,
    WIFI_NETWORK_VALID_NOT_AVAILABLE = 2,
    WIFI_NETWORK_VALID_AVAILABLE = 3,
} wifi_networks_status_e;


#define XWIFI_EVENT_SCAN_DONE BIT0
#define XWIFI_EVENT_NO_AP_FOUND BIT1
#define XWIFI_EVENT_WRONG_PASSWORD BIT2
#define XWIFI_EVENT_STA_DISCONNECTED BIT3
#define XWIFI_EVENT_SCAN_COMMAND BIT4
#define XWIFI_EVENT_LOOK_FOR_NETWORK BIT5

#define WIFI_SSID_LEN 32
#define WIFI_PASS_LEN 64

typedef struct {
    uint8_t ssid[WIFI_SSID_LEN];
    uint8_t password[WIFI_PASS_LEN];
    int8_t rssi;
    wifi_networks_status_e status;
} wifi_networks_t;

static esp_netif_t *wifi_netif = NULL;
static esp_netif_t *ap_netif = NULL;
// Normalde bunu scan içinde yerel değişken yapıp NVS üzerinden değerleri okumalıyız!
wifi_networks_t wifi_networks[WIFI_NETWORKS_MAX] = {
    {"MonsterHotspot", "12345678", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID2", "PASSWORD2", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID3", "PASSWORD3", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID4", "PASSWORD4", 0, WIFI_NETWORK_NOT_VALID},
    {"", "", 0, WIFI_NETWORK_NOT_VALID},
    {"", "", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID7", "PASSWORD7", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID8", "PASSWORD8", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID9", "PASSWORD9", 0, WIFI_NETWORK_NOT_VALID},
    {"SSID10", "PASSWORD10", 0, WIFI_NETWORK_NOT_VALID},
};

//TODO: Bunun için default initizalizer makrosu yapılacak! 

// RTOS Specific

static EventGroupHandle_t wifi_event_group;


// Event üretmek gerekli, event group içinde denetlemeliyiz.
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
                wifi_event_sta_disconnected_t *status = (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGW("WIFI EVENT", "WIFI_EVENT_STA_DISCONNECTED Reason-> %d", status->reason);
                switch(status->reason)
                {   
                    // Bu noktada her bir networke id vermek gerekli!
                    case WIFI_REASON_NO_AP_FOUND: // Kapsama alanı dışında 
                    ESP_LOGW("WIFI EVENT", "No AP Found");
                    xEventGroupSetBits(wifi_event_group, XWIFI_EVENT_NO_AP_FOUND);
                    break;

                    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: // Şifre yanlış
                    ESP_LOGW("WIFI EVENT", "Wrong Password 4WAY HANDSHAKE TIMEOUT");
                    xEventGroupSetBits(wifi_event_group, XWIFI_EVENT_WRONG_PASSWORD);
                    break;
                }
                if (status->reason == 15)
                {
                    //wifi credentials are incorrect
                }

                xEventGroupSetBits(wifi_event_group, XWIFI_EVENT_STA_DISCONNECTED);
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

            case WIFI_EVENT_SCAN_DONE: //! Tarama Tamamlandı! (Bu tasktan event çıkar)
            {
                ESP_LOGI("WIFI_EVENT", "WIFI_EVENT_SCAN_DONE");
                xEventGroupSetBits(wifi_event_group, XWIFI_EVENT_SCAN_DONE);
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

            case IP_EVENT_STA_LOST_IP:
            {
                ESP_LOGI("WIFI", "Lost IP");
                LED_OFF(LED_GREEN);
            }
            break;

            case IP_EVENT_AP_STAIPASSIGNED:
            {
                ESP_LOGI("WIFI", "AP Assigned to IP to connected station");
            }
            break;   
        }
    } 

}



static void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Bu kısımda networklerin geçerli olup olmadığı kontrol edilecek

    for (int i = 0; i < WIFI_NETWORKS_MAX; i++)
    {
        if((strnlen((char*)wifi_networks[i].password, WIFI_PASS_LEN) >= 8) && (strnlen((char*)wifi_networks[i].ssid, WIFI_SSID_LEN) >= 2))
        {
            ESP_LOGW("WIFI", "Network %d is valid!", i);
            wifi_networks[i].status = WIFI_NETWORK_VALID;
        }
        else
        {
            ESP_LOGW("WIFI", "Network %d is not valid!", i);
            wifi_networks[i].status = WIFI_NETWORK_NOT_VALID;
        }
    }
}

static void wifi_scan_mode() // Bu fonksiyon tarama işini başlatıp eventı beklemek için kullanılır
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
    esp_wifi_scan_start(&scan_config, false); // Non-blocking
}

void wifi_go_offline(void)
{
    // Burada wifi ayarları da sıfırlanacak
    esp_wifi_disconnect();
    esp_wifi_restore();
    esp_wifi_stop();
}


static void wifi_sta_mode(wifi_networks_t* network)
{
    wifi_go_offline();
    wifi_config_t sta_config = {
        .sta = { //? Bu ayarları boş bırakınca bağlanmıyor, incele!
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
            .sae_h2e_identifier = ""
        },
    };
    strncpy((char*)sta_config.sta.ssid, (char*)network->ssid, 32);
    strncpy((char*)sta_config.sta.password, (char*)network->password, 64);


    ESP_LOGW("WIFI", "STA SSID: %s", sta_config.sta.ssid);
    ESP_LOGW("WIFI", "STA PASS: %s", sta_config.sta.password);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    esp_wifi_start();
}

static void wifi_ap_mode(void)
{
    wifi_go_offline();
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

// Tasklist

void wifi_task(void *pvParameters)
{
    wifi_event_group = xEventGroupCreate();
    static wifi_networks_t* chosen_network = &wifi_networks[0]; // 0 numaralı network default olarak seçili
    while (1)
    {

        //* Düğme okumalar
        if(!BUTTON1_READ())
        {
            wifi_scan_mode();
            LED_ON(LED_RED);
            LED_ON(LED_GREEN);
            vTaskDelay(pdMS_TO_TICKS(100));
            LED_ALL_OFF();
        }

        if(!BUTTON2_READ())
        {
            wifi_go_offline();
            if(chosen_network != NULL)
            {
                wifi_sta_mode(chosen_network);
            }
            else
            {
                LED_ALL_OFF();
                LED_ON(LED_RED);
                ESP_LOGE("WIFI", "Chosen network is null!");
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        //* Event okumalar
        EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
        if(uxBits)
        {
            if (uxBits & XWIFI_EVENT_SCAN_COMMAND)
            {
                wifi_scan_mode();
                LED_ON(LED_RED);
                LED_ON(LED_GREEN);
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_SCAN_COMMAND);
            }
            else if (uxBits & XWIFI_EVENT_SCAN_DONE) // Çoklu event söz konusu olabilir, maskeleme gerek. 
            {
                ESP_LOGI("RTOS", "Wifi Event Alındı! EVENT_SCAN_DONE");
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_SCAN_DONE);
                uint16_t ap_count = 0;
                esp_wifi_scan_get_ap_num(&ap_count);
                if (ap_count == 0)
                {
                    ESP_LOGW("WIFI", "No AP found!");
                }
                else
                {
                    wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_list));
                    //! RTOS'da for kullanmak?
                    for (int i = 0; (i < ap_count) && (i < WIFI_NETWORKS_MAX); i++)
                    {
                        ESP_LOGI("WIFI", "SSID: %s, RSSI: %d", ap_list[i].ssid, ap_list[i].rssi);
                        for (int j = 0; j < WIFI_NETWORKS_MAX; j++)
                        {
                            if(wifi_networks[j].status == WIFI_NETWORK_NOT_VALID || wifi_networks[j].status == WIFI_NETWORK_VALID_AVAILABLE)
                            {
                                continue;
                            }
                            if (strncmp((char*)ap_list[i].ssid, (char*)wifi_networks[j].ssid, WIFI_SSID_LEN) == 0)
                            {
                                ESP_LOGI("WIFI", "Matched Network -> SSID: %s, SSID: %s", wifi_networks[j].ssid, ap_list[i].ssid);
                                wifi_networks[j].rssi = ap_list[i].rssi;
                                wifi_networks[j].status = WIFI_NETWORK_VALID_AVAILABLE;
                            }
                            else
                            {
                                ESP_LOGI("WIFI", "Not Matched Network -> SSID: %s, SSID: %s", wifi_networks[j].ssid, ap_list[i].ssid);
                                wifi_networks[j].status = WIFI_NETWORK_VALID_NOT_AVAILABLE;
                                wifi_networks[j].rssi = 0;
                            }
                        }
                    }
                    free(ap_list);

                    // Show matched APs
                    for (int i = 0; i < WIFI_NETWORKS_MAX; i++)
                    {
                        if(wifi_networks[i].status == WIFI_NETWORK_VALID_AVAILABLE)
                        {
                            ESP_LOGI("WIFI", "Available Network -> SSID: %s, RSSI: %d", wifi_networks[i].ssid, wifi_networks[i].rssi);
                        }
                        else
                        {
                            ESP_LOGW("WIFI", "Not Available Network -> Status: %d, index: %d", wifi_networks[i].status, i);
                        }
                    }

                }
                // Bu kısımda veriyi alıp ayıklama işini yapacağız. 
                // Sort available networks by rssi
                int8_t max_rssi = -100;
                int8_t max_rssi_index = -1;
                for(int i = 0; i < WIFI_NETWORKS_MAX; i++)
                {
                    if (wifi_networks[i].status == WIFI_NETWORK_VALID_AVAILABLE)
                    {
                        if(wifi_networks[i].rssi > max_rssi)
                        {
                            max_rssi = wifi_networks[i].rssi;
                            max_rssi_index = i;
                        }
                    }
                }
                if (max_rssi_index != -1)
                {
                    chosen_network = &wifi_networks[max_rssi_index];
                    ESP_LOGI("WIFI", "Chosen Network -> SSID: %s, RSSI: %d", chosen_network->ssid, chosen_network->rssi);
                }
                else
                {
                    ESP_LOGW("WIFI", "No available network found!");
                }
            }
            else if (uxBits & XWIFI_EVENT_NO_AP_FOUND)
            {
                // Bu noktada bir sonraki taramaya kadar kullanılmayacak. 
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_NO_AP_FOUND);
                chosen_network->status = WIFI_NETWORK_VALID_NOT_AVAILABLE;

            }
            else if (uxBits & XWIFI_EVENT_WRONG_PASSWORD)
            {   // Bu noktada yanlış şifreli AP tarama dışına atılmakta, bir sonraki captive portalda uyarı olarak gösterilecek
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_WRONG_PASSWORD);
                chosen_network->status = WIFI_NETWORK_NOT_VALID;
            }
            else if (uxBits & XWIFI_EVENT_STA_DISCONNECTED)
            {
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_STA_DISCONNECTED);
                ESP_LOGI("RTOS", "Wifi Event Alındı! STA_DISCONNECTED");
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_STA_DISCONNECTED);
                ESP_LOGI("WIFI", "Wifi disconnected!");
                wifi_go_offline();
                // Burada NO_AP found kapsama alanı dışında da olmakta!

                // Eğer NO_AP found değilse aynı AP'ye bir daha bağlanmayı dene (NO_AP alana kadar deneyebilir)
                switch (chosen_network->status)
                {
                    case WIFI_NETWORK_VALID_AVAILABLE:
                    {
                        wifi_sta_mode(chosen_network);
                    }
                    break;

                    case WIFI_NETWORK_NOT_VALID:
                    case WIFI_NETWORK_VALID_NOT_AVAILABLE:
                    {
                        xEventGroupSetBits(wifi_event_group, XWIFI_EVENT_LOOK_FOR_NETWORK);
                    }
                    break; 

                    default:
                    break;  
                }
            }
            else if (uxBits & XWIFI_EVENT_LOOK_FOR_NETWORK)
            {
                xEventGroupClearBits(wifi_event_group, XWIFI_EVENT_LOOK_FOR_NETWORK);
                for(int i = 0; i < WIFI_NETWORKS_MAX; i++)
                {
                    if(wifi_networks[i].status == WIFI_NETWORK_VALID_AVAILABLE)
                    {
                        chosen_network = &wifi_networks[i];
                        break;
                    }
                }

                if (chosen_network->status != WIFI_NETWORK_VALID_AVAILABLE)
                {
                    ESP_LOGW("WIFI", "No available network found to connect!");
                    xEventGroupSetBits(wifi_event_group, XWIFI_EVENT_SCAN_COMMAND); // Scan for networks next time
                    wifi_go_offline();
                }
                else
                {
                    ESP_LOGI("WIFI", "Chosen Network -> SSID: %s, RSSI: %d", chosen_network->ssid, chosen_network->rssi);
                    wifi_sta_mode(chosen_network);
                }
            }

            else
            {
                ESP_LOGI("RTOS", "Wifi Diğer Event!");
            }
        }

        vTaskDelay((TickType_t)1);
    }

    vTaskDelete(NULL);
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

    xTaskCreate(wifi_task, "wifi_scan_mode", 4096, NULL, 1, NULL);
    while(1)
    {

        vTaskDelay((TickType_t)1);
    }
}


/*

static int find_max_rssi_index()
{
    int8_t max_rssi = -100;
    int8_t max_rssi_index = -1;
    for(int i = 0; i < WIFI_NETWORKS_MAX; i++)
    {
        if (wifi_networks[i].status == WIFI_NETWORK_VALID_AVAILABLE)
        {
            if(wifi_networks[i].rssi > max_rssi)
            {
                max_rssi = wifi_networks[i].rssi;
                max_rssi_index = i;
            }
        }
    }
    if (max_rssi_index != -1)
    {
        ESP_LOGI("WIFI", "Max RSSI Index: %d", max_rssi_index);
    }
    else
    {
        ESP_LOGW("WIFI", "No available network found!");
    }

    return max_rssi_index;
}

*/