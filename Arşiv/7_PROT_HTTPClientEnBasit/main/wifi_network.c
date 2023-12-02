#include "wifi_network.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
// Private Macros

#define return_on_err(err) if(err != ESP_OK) return err;
#define deinit_on_err(err) if(err != ESP_OK) {esp_wifi_deinit(); return err;}

#if (WIFI_NETWORK_DEBUG_LOG)
#define check_null_ptr(ptr) if(ptr == NULL) {ESP_LOGE("x", "Null Pointer"); return ESP_ERR_INVALID_ARG;}
#else
#define check_null_ptr(ptr) if(ptr == NULL) return ESP_ERR_INVALID_ARG
#endif
// Private Globals 

static wifi_network_info_t wifi_network_info = WIFI_NETWORK_DEFAULT_CONFIG();
static esp_netif_t *wifi_netif = NULL;
static esp_netif_t *ap_netif = NULL;
static wifi_network_status_e wifi_network_state = WIFI_NETWORK_STATE_DEINIT;
static wifi_network_sta_connection_status_e wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_NOTAVAILABLE;
static wifi_network_ap_connection_status_e wifi_ap_connection_state = WIFI_NETWORK_AP_STATE_NOTAVAILABLE;
static wifi_network_disconnect_reason_e wifi_disconnect_reason = WIFI_NETWORK_DISCONNECT_REASON_NOT_DEFINED;
static EventGroupHandle_t wifi_network_event_group = NULL;
// Private typedefs 

// Private Defines 

#define WIFI_EVENT_ERROR 0xFFFF
#define COMMON_MSG_SIZE 50
#define ERROR_MSG_SIZE 50

#define WIFI_NETWORK_OK 0
#define WIFI_NETWORK_ERROR -1


// Private Func Prototypes

static err_t secure_string_copy(char *dest, char *src, size_t dest_size);
static error_check_level_e err_to_level(esp_err_t err);
static void err_to_string(esp_err_t err, char * msg);
static esp_err_t wifi_error_check(esp_err_t err, char *user_msg);
static void reason_to_string(wifi_network_disconnect_reason_e reason, char* reason_string);

// Error codes
/*
#define ESP_FAIL                                         (-1) // Generic esp_err_t code indicating failure
#define ESP_OK                                           (0) // esp_err_t value indicating success (no error)
#define ESP_ERR_NO_MEM                                   (0x101) // Out of memory
#define ESP_ERR_INVALID_ARG                              (0x102) // Invalid argument
#define ESP_ERR_INVALID_STATE                            (0x103) // Invalid state
#define ESP_ERR_INVALID_SIZE                             (0x104) // Invalid size
#define ESP_ERR_NOT_FOUND                                (0x105) // Requested resource not found
#define ESP_ERR_NOT_SUPPORTED                            (0x106) // Operation or feature not supported
#define ESP_ERR_TIMEOUT                                  (0x107) // Operation timed out
#define ESP_ERR_INVALID_RESPONSE                         (0x108) // Received response was invalid
#define ESP_ERR_INVALID_CRC                              (0x109) // CRC or checksum was invalid
#define ESP_ERR_INVALID_VERSION                          (0x10a) // Version was invalid
#define ESP_ERR_INVALID_MAC                              (0x10b) // MAC address was invalid
#define ESP_ERR_NOT_FINISHED                             (0x10c) // Operation has not fully completed
#define ESP_ERR_NOT_ALLOWED                              (0x10d) // Operation is not allowed
*/ 

#define WIFI_NETWORK_ERR_WIFI_NOT_INIT                   (0x3001) // WiFi driver was not installed by esp_wifi_init
#define WIFI_NETWORK_ERR_WIFI_NOT_STARTED                (0x3002) // WiFi driver was not started by esp_wifi_start
#define WIFI_NETWORK_WIFI_NOT_STOPPED                    (0x3003) // WiFi driver was not stopped by esp_wifi_stop
#define WIFI_NETWORK_ERR_WIFI_IF                         (0x3004) // WiFi interface error
#define WIFI_NETWORK_ERR_WIFI_MODE                       (0x3005) // WiFi mode error
#define WIFI_NETWORK_ERR_WIFI_STATE                      (0x3006) // WiFi internal state error
#define WIFI_NETWORK_ERR_WIFI_CONN                       (0x3007) // WiFi internal control block of station or soft-AP error
#define WIFI_NETWORK_ERR_WIFI_NVS                        (0x3008) // WiFi internal NVS module error
#define WIFI_NETWORK_ERR_WIFI_MAC                        (0x3009) // MAC address is invalid
#define WIFI_NETWORK_ERR_WIFI_SSID                       (0x300a) // SSID is invalid
#define WIFI_NETWORK_ERR_WIFI_PASSWORD                   (0x300b) // Password is invalid
#define WIFI_NETWORK_ERR_WIFI_TIMEOUT                    (0x300c) // Timeout error
#define WIFI_NETWORK_ERR_WIFI_WAKE_FAIL                  (0x300d) // WiFi is in sleep state(RF closed) and wakeup fail
#define WIFI_NETWORK_ERR_WIFI_WOULD_BLOCK                (0x300e) // The caller would block
#define WIFI_NETWORK_ERR_WIFI_NOT_CONNECT                (0x300f) // Station still in disconnect status
#define WIFI_NETWORK_ERR_WIFI_POST                       (0x3012) // Failed to post the event to WiFi task
#define WIFI_NETWORK_ERR_WIFI_INIT_STATE                 (0x3013) // Invalid WiFi state when init/deinit is called
#define WIFI_NETWORK_ERR_WIFI_STOP_STATE                 (0x3014) // Returned when WiFi is stopping
#define WIFI_NETWORK_ERR_WIFI_NOT_ASSOC                  (0x3015) // The WiFi connection is not associated
#define WIFI_NETWORK_ERR_WIFI_TX_DISALLOW                (0x3016) // The WiFi TX is disallowed
#define WIFI_NETWORK_ERR_WIFI_REGISTRAR                  (0x3033) // WPS registrar is not supported
#define WIFI_NETWORK_ERR_WIFI_WPS_TYPE                   (0x3034) // WPS type error
#define WIFI_NETWORK_ERR_WIFI_WPS_SM                     (0x3035) // WPS state machine is not initialized

#define WIFI_NETWORK_ERR_ESP_NETIF_INVALID_PARAMS        (0x5001)
#define WIFI_NETWORK_ERR_ESP_NETIF_IF_NOT_READY          (0x5002)
#define WIFI_NETWORK_ERR_ESP_NETIF_DHCPC_START_FAILED    (0x5003)
#define WIFI_NETWORK_ERR_ESP_NETIF_DHCP_ALREADY_STARTED  (0x5004)
#define WIFI_NETWORK_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED  (0x5005)
#define WIFI_NETWORK_ERR_ESP_NETIF_NO_MEM                (0x5006)
#define WIFI_NETWORK_ERR_ESP_NETIF_DHCP_NOT_STOPPED      (0x5007)
#define WIFI_NETWORK_ERR_ESP_NETIF_DRIVER_ATTACH_FAILED  (0x5008)
#define WIFI_NETWORK_ERR_ESP_NETIF_INIT_FAILED           (0x5009)
#define WIFI_NETWORK_ERR_ESP_NETIF_DNS_NOT_CONFIGURED    (0x500a)




static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{

    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            // STA EVENTS
            /* Bu Event 
            esp_wifi_scan_start() 
            esp_wifi_scan_stop()
            taramadayken esp_wifi_scan_start() ile tetiklenmektedir
            esp_wifi_connect() ve blocked scanda bu olay gerçekleşmez*/
            case WIFI_EVENT_SCAN_DONE: 
            {
                
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_SCAN_DONE");
                #endif
                wifi_event_sta_scan_done_t *scan_done_event_data = (wifi_event_sta_scan_done_t*) event_data;
                // status 0 success, number of AP, scan sequence number (?)
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_SCAN_DONE);
                wifi_network_offline();
            }
            break;

           // Bu olay esp_wifi_start() fonksiyonu ESP_OK dördüğünde tetiklenir.
            case WIFI_EVENT_STA_START:
            {
                wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_AVAILABLE;
                ESP_LOGW("WIFI NETWORK", "Address of wifi_network_event_group %p", wifi_network_event_group);
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_START");
                #endif

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_START);
            }
            break;

            // Bu olay esp_wifi_stop() fonksiyonu ESP_OK döndürdüğünde tetiklenmektedir.
            case WIFI_EVENT_STA_STOP:
            {
                wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_NOTAVAILABLE;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_STOP");
                #endif

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_STOP);
            }
            break;

            // Bu event esp_wifi_connect() ESP_OK döndürdüğünde tetiklenir. Bağlantı sağlandığında yani.
            // Eğer uygulama LwIP tabanlıysa IP adresini almasını da beklemek gerekli.
            case WIFI_EVENT_STA_CONNECTED:
            {
                wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_CONNECTED;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_CONNECTED");
                #endif
                wifi_event_sta_connected_t *sta_connected_event_data = (wifi_event_sta_connected_t*) event_data;
                // ssid, ssid_len, bssid, channel, authmode, aid 

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_CONNECTED);
            }
            break;

            // AP'ye bağlı iken esp_wifi_disconnect() veya esp_wifi_stop() çağırıldığında
            // esp_wifi_connect() çağırılıp bağlantı gerçekleşemediğinde (reason)
            // AP tarafından bağlantı kesildiğinde
            //* BU DURUMUN DİKKATLİ BİR ŞEKİLDE ELE ALINMASI GEREKLI DOC OKU
            // ! Bu noktada bütün socket/protokol işlemleri kapatılmalıdır!
            case WIFI_EVENT_STA_DISCONNECTED:
            {
                wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_DISCONNECTED;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_DISCONNECTED");
                #endif
                wifi_event_sta_disconnected_t *sta_disconnected_event_data = (wifi_event_sta_disconnected_t*) event_data;
                // Reason burada switch case yapılacak
                // wifi_err_reason_t tipinde
                // ssid, ssid_len, bssid, channel, authmode, aid 
                wifi_disconnect_reason = sta_disconnected_event_data->reason;
                #if (WIFI_NETWORK_DEBUG_LOG)
                    char common_msg[COMMON_MSG_SIZE] = "\0";
                    reason_to_string(wifi_disconnect_reason, common_msg);
                    ESP_LOGW("WIFI_NETWORK", "Disconnect Reason: %s", common_msg);
                #endif
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_DISCONNECTED);
            }
            break;

            case WIFI_EVENT_STA_AUTHMODE_CHANGE:
            {
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_AUTHMODE_CHANGE");
                #endif
                wifi_event_sta_authmode_change_t *sta_authmode_change_event_data = (wifi_event_sta_authmode_change_t*) event_data;
                // wifi_auth_mode_t old_mode ve new_mode 
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_AUTHMODE_CHANGE);
            }
            break;

            //AP EVENTS
            // STA Start eventıne benzerdir. 
            case WIFI_EVENT_AP_START:
            {
                wifi_ap_connection_state = WIFI_NETWORK_AP_STATE_AVAILABLE;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_AP_START");
                #endif
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_AP_START);
            }
            break;

            // STA Stop eventine benzerdir. 
            case WIFI_EVENT_AP_STOP:
            {
                wifi_ap_connection_state = WIFI_NETWORK_AP_STATE_NOTAVAILABLE;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_AP_STOP");
                #endif
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_AP_STOP);
            }
            break;

            // Her istasyon bağlantığında bu olay tetiklenir. 
            case WIFI_EVENT_AP_STACONNECTED:
            {
                wifi_ap_connection_state = WIFI_NETWORK_AP_STATE_CLIENT_CONNECTED;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_AP_STACONNECTED");
                #endif
                wifi_event_ap_staconnected_t *ap_staconnected_event_data = (wifi_event_ap_staconnected_t*) event_data;
                // mac, aid, is_mest_child
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_AP_STACONNECTED);
            }
            break;

            // Bu olay esp_wifi_disconnect() veya esp_wifi_deauth_sta() veya manuel olarak çıkıldığında tetiklenir
            // esp_wifi_set_inactive_time() ile ayarlanan bağlı STA 5dk paket göndermezse atılır

            case WIFI_EVENT_AP_STADISCONNECTED:
            {
                wifi_ap_connection_state = WIFI_NETWORK_AP_STATE_CLIENT_DISCONNECTED;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_AP_STADISCONNECTED");
                #endif
                wifi_event_ap_stadisconnected_t *ap_stadisconnected_event_data = (wifi_event_ap_stadisconnected_t*) event_data;
                // mac, aid, is_mest_child, reason 
                // Reason burada switch case yapılacak
                // wifi_err_reason_t tipinde
                wifi_disconnect_reason = ap_stadisconnected_event_data->reason;
                #if (WIFI_NETWORK_DEBUG_LOG)
                    char common_msg[COMMON_MSG_SIZE] = "\0";
                    reason_to_string(wifi_disconnect_reason, common_msg);
                    ESP_LOGW("WIFI_NETWORK", "Disconnect Reason: %s", common_msg);
                #endif
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_AP_STADISCONNECTED);
            }
            break;

            // Bu event esp_wifi_set_event_mask() ile etkinleştirilebilir. 
            case WIFI_EVENT_AP_PROBEREQRECVED:
            {
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_AP_PROBEREQRECVED");
                #endif

                wifi_event_ap_probe_req_rx_t *ap_probe_event_data = (wifi_event_ap_probe_req_rx_t*) event_data;
                // rssi, mac 

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_AP_PROBEREQRECVED);
            }
            break;


            case WIFI_EVENT_STA_BSS_RSSI_LOW:
            {
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_BSS_RSSI_LOW");
                #endif

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_BSS_RSSI_LOW);
            }
            break;

            // AP'den bir paket gelm(?)
            case  WIFI_EVENT_STA_BEACON_TIMEOUT:
            {
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "WIFI_EVENT_STA_BEACON_TIMEOUT");
                #endif

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_STA_BEACON_TIMEOUT);
            }
            break;

            // Errro

            case WIFI_EVENT_ERROR:
            {
                ESP_LOGE("WIFI_NETWORK", "WIFI_EVENT_ERROR");

                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_EVENT_ERROR);
            }
            break;


            default:
                // Bilinmeyen event için bit oluşturmalı
                ESP_LOGW("WIFI_NETWORK", "Unknown WIFI_EVENT: %li", event_id);
                xEventGroupSetBits(wifi_network_event_group, WIFI_NETWORK_EVENT_BIT_UNKNOWN_EVENT);
            break;
        }

    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
            // DHCP'den başarıyla IP alındığında veya IP adresi değiştiğinde bu olay tetiklenir. 
            //!Bu olaydan önce protokol/socket işlemleri yapılmamalı!!
            //!ip_change true ise IP değişmiş demektir, socket/protokol işlemlerini sıfırlamalı
            case IP_EVENT_STA_GOT_IP: // station got IP from connected AP
            {
                wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_GOTIP;
                ip_event_got_ip_t *got_ip_event_data = (ip_event_got_ip_t*) event_data;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "IP_EVENT_STA_GOT_IP");
                #endif
            }
            break;

            case IP_EVENT_STA_LOST_IP: // station lost IP and the IP is reset to 0
            {
                wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_CONNECTED;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "IP_EVENT_STA_LOST_IP");
                #endif
            }
            break;

            case IP_EVENT_AP_STAIPASSIGNED: // soft-AP assign an IP to a connected station
            {
                wifi_ap_connection_state = WIFI_NETWORK_AP_STATE_CLIENT_GOT_IP;
                ip_event_ap_staipassigned_t *ap_staipassigned_event_data = (ip_event_ap_staipassigned_t*) event_data;
                #if (WIFI_NETWORK_DEBUG_LOG)
                ESP_LOGI("WIFI_NETWORK", "IP_EVENT_AP_STAIPASSIGNED");
                #endif
            }
            break;
            /* Kullanılmayan eventlar
            IP_EVENT_GOT_IP6 // station or ap or ethernet interface v6IP addr is preferred
            IP_EVENT_ETH_GOT_IP // ethernet got IP from connected AP
            IP_EVENT_ETH_LOST_IP // ethernet lost IP and the IP is reset to 0
            IP_EVENT_PPP_GOT_IP // PPP interface got IP
            IP_EVENT_PPP_LOST_IP // PPP interface lost IP
            */
            default:
            break;
        }
        
    }
}


//*------------------------------ API Functions -----------------------------------------*//

esp_err_t wifi_network_config(wifi_network_info_t *config_info) // Burada temel wifi configurasyonları ile runtime konfigürasyonları yapılacak
{
    nw_err_t err = ESP_OK;

    err = wifi_network_set_sta_ssid((char*)config_info->sta_ssid, WIFI_NETWORK_MAX_SSID_LEN);
    wifi_error_check(err, "func: wifi_network_config() -> wifi_network_set_sta_ssid()");

    err = wifi_network_set_sta_pass((char*)config_info->sta_pass, WIFI_NETWORK_MAX_PASS_LEN);
    wifi_error_check(err, "func: wifi_network_config() -> wifi_network_set_sta_pass()");

    err = wifi_network_set_ap_ssid((char*)config_info->ap_ssid, WIFI_NETWORK_MAX_SSID_LEN);
    wifi_error_check(err, "func: wifi_network_config() -> wifi_network_set_ap_ssid()");

    err = wifi_network_set_ap_pass((char*)config_info->ap_pass, WIFI_NETWORK_MAX_PASS_LEN);
    wifi_error_check(err, "func: wifi_network_config() -> wifi_network_set_ap_pass()");

    return err;
}

esp_err_t wifi_network_init(EventGroupHandle_t event_group)
{
    esp_err_t err = ESP_OK;

    check_null_ptr(event_group);
    wifi_network_event_group = event_group;

    err = esp_netif_init();
    wifi_error_check(err, "func: wifi_network_init() -> esp_netif_init()");
    return_on_err(err);

    err = esp_event_loop_create_default();
    wifi_error_check(err, "func: wifi_network_init() -> esp_event_loop_create_default()");
    return_on_err(err);

    wifi_netif = esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    wifi_error_check(err, "func: wifi_network_init() -> esp_event_handler_instance_register()");
    return_on_err(err);

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    wifi_error_check(err, "func: wifi_network_init() -> esp_event_handler_instance_register()");
    return_on_err(err);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    wifi_error_check(err, "func: wifi_network_init() -> esp_wifi_init()");
    return_on_err(err);

    ESP_LOGI("WIFI_NETWORK", "wifi_network_init finished. err: %d", err);
    wifi_network_state = WIFI_NETWORK_STATE_INIT;
    return err;
}


esp_err_t wifi_network_station_mode(void)
{
    esp_err_t err = ESP_OK;

    wifi_network_offline();

    wifi_config_t sta_config = {
        .sta = { //? Bu ayarları boş bırakınca bağlanmıyor, incele!
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
            .sae_h2e_identifier = ""
        },
    };

    err = secure_string_copy((char*)sta_config.sta.ssid, (char*)wifi_network_info.sta_ssid, sizeof(sta_config.sta.ssid));
    wifi_error_check(err, "func: wifi_network_station_mode() -> secure_string_copy()");

    err = secure_string_copy((char*)sta_config.sta.password, (char*)wifi_network_info.sta_pass, sizeof(sta_config.sta.password));
    wifi_error_check(err, "func: wifi_network_station_mode() -> secure_string_copy()");

#if(WIFI_NETWORK_DEBUG_LOG)
    ESP_LOGI("WIFI", "STA SSID: %s", sta_config.sta.ssid);
    ESP_LOGI("WIFI", "STA PASS: %s", sta_config.sta.password);
#endif

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    wifi_error_check(err, "func: wifi_network_station_mode() -> esp_wifi_set_mode()");
    return_on_err(err);
    err = esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    wifi_error_check(err, "func: wifi_network_station_mode() -> esp_wifi_set_config()");
    return_on_err(err);
    err = esp_wifi_start();
    wifi_error_check(err, "func: wifi_network_station_mode() -> esp_wifi_start()");
    return_on_err(err);
    
    err = esp_wifi_connect();
    wifi_error_check(err, "func: wifi_network_station_mode() -> esp_wifi_connect()");
    return_on_err(err);

    ESP_LOGI("WIFI_NETWORK", "wifi_network_station_mode finished. err: %d", err);

    wifi_network_state = WIFI_NETWORK_STATE_STA;
    return err;
}

esp_err_t wifi_network_softap_mode(void)
{
    esp_err_t err = ESP_OK;

    wifi_network_offline();
    wifi_config_t ap_config = {
        .ap = {
            .ssid_len = WIFI_NETWORK_MAX_SSID_LEN,
            .channel = WIFI_NETWORK_DEFAULT_AP_CHANNEL,
            .max_connection = WIFI_NETWORK_DEFAULT_AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = true
            },
        },
    };

    err = secure_string_copy((char*)ap_config.ap.ssid, (char*)wifi_network_info.ap_ssid, sizeof(ap_config.ap.ssid));
    wifi_error_check(err, "func: wifi_network_softap_mode() -> secure_string_copy()");

    if (strlen((char*)ap_config.ap.ssid) == 0)
    {
        #if (WIFI_NETWORK_AP_SSID_MUST_HAVE)
            ESP_LOGE("WIFI_NETWORK", "AP SSID is empty, AP_SSID_MUST_HAVE");
            return ESP_ERR_INVALID_ARG;
        #else
            strncpy((char*)ap_config.ap.ssid, WIFI_NETWORK_DEFAULT_AP_SSID, WIFI_NETWORK_MAX_SSID_LEN);
        #endif
    }

    err = secure_string_copy((char*)ap_config.ap.password, (char*)wifi_network_info.ap_pass, sizeof(ap_config.ap.password));
    wifi_error_check(err, "func: wifi_network_softap_mode() -> secure_string_copy()");

    if (strnlen((char*)ap_config.ap.password, WIFI_NETWORK_MAX_PASS_LEN) == 0)
    {
        #if (WIFI_NETWORK_AP_PASSWORD_MUST_HAVE)
            ESP_LOGE("WIFI_NETWORK", "AP SSID is empty, AP_PASSWORD_MUST_HAVE");
            return ESP_ERR_INVALID_ARG;
        #else
            ap_config.ap.authmode = WIFI_AUTH_OPEN;
        #endif
    }

#if(WIFI_NETWORK_DEBUG_LOG)
    ESP_LOGI("WIFI", "AP SSID: %s", ap_config.ap.ssid);
    ESP_LOGI("WIFI", "AP PASS: %s", ap_config.ap.password);
#endif

    err = esp_wifi_set_mode(WIFI_MODE_AP);
    wifi_error_check(err, "func: wifi_network_softap_mode() -> esp_wifi_set_mode()");
    return_on_err(err);

    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config);
    wifi_error_check(err, "func: wifi_network_softap_mode() -> esp_wifi_set_config()");
    return_on_err(err);

    err = esp_wifi_start();
    wifi_error_check(err, "func: wifi_network_softap_mode() -> esp_wifi_start()");
    return_on_err(err);

    wifi_network_state = WIFI_NETWORK_STATE_AP;
    return err;
}

esp_err_t wifi_network_scan_mode(void)
{

    esp_err_t err = ESP_OK; 

    err = esp_wifi_stop();
    wifi_error_check(err, "func: wifi_network_scan_mode() -> esp_wifi_stop()");
    return_on_err(err);

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    wifi_error_check(err, "func: wifi_network_scan_mode() -> esp_wifi_set_mode()");
    return_on_err(err);

    err = esp_wifi_start();
    wifi_error_check(err, "func: wifi_network_scan_mode() -> esp_wifi_start()");
    return_on_err(err);
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
    err = esp_wifi_scan_start(&scan_config, false); // Non-blocking
    wifi_sta_connection_state = WIFI_NETWORK_STA_STATE_SCANNING;
    wifi_network_state = WIFI_NETWORK_STATE_SCAN;
    return err;
}

esp_err_t wifi_network_offline(void)
{
    esp_err_t err = ESP_OK;

    if (wifi_network_state == WIFI_NETWORK_STATE_STA && wifi_sta_connection_state != WIFI_NETWORK_STA_STATE_NOTAVAILABLE)
    {
        err = esp_wifi_disconnect();
        wifi_error_check(err, "func: wifi_network_offline() -> esp_wifi_disconnect()");
    }

    err = esp_wifi_restore();
    wifi_error_check(err, "func: wifi_network_offline() -> esp_wifi_restore()");

    
    err = esp_wifi_stop();
    wifi_error_check(err, "func: wifi_network_offline() -> esp_wifi_stop()");
    
    wifi_network_state = WIFI_NETWORK_STATE_INIT;
    return err;
}

esp_err_t wifi_network_deinit(void)
{
    esp_err_t err = ESP_OK;

    wifi_network_offline();
    esp_wifi_deinit();

    wifi_network_state = WIFI_NETWORK_STATE_DEINIT;
    return err;
}


//*-------------------------------------- GET-SET Functions-----------------------------------*//

// Set Functions 

nw_err_t wifi_network_set_sta_ssid(char* sta_ssid, size_t ssid_len)
{
    nw_err_t err;
    err = secure_string_copy((char*)wifi_network_info.sta_ssid, sta_ssid, ssid_len);
    return err;
}

nw_err_t wifi_network_set_sta_pass(char* sta_pass, size_t pass_len)
{
    nw_err_t err;
    err = secure_string_copy((char*)wifi_network_info.sta_pass, sta_pass, pass_len);
    return err;
}

nw_err_t wifi_network_set_ap_ssid(char* ap_ssid, size_t ssid_len)
{
    nw_err_t err;
    err = secure_string_copy((char*)wifi_network_info.ap_ssid, ap_ssid, ssid_len);
    return err;
}

nw_err_t wifi_network_set_ap_pass(char* ap_pass, size_t pass_len)
{
    nw_err_t err;
    err = secure_string_copy((char*)wifi_network_info.ap_pass, ap_pass, pass_len);
    return err;
}

// Get Functions 

nw_err_t wifi_network_get_sta_ssid(char* sta_ssid, size_t ssid_len)
{
    nw_err_t err;
    err = secure_string_copy(sta_ssid, (char*)wifi_network_info.sta_ssid, ssid_len);
    return err;
}

nw_err_t wifi_network_get_sta_pass(char* sta_pass, size_t pass_len)
{
    nw_err_t err;
    err = secure_string_copy(sta_pass, (char*)wifi_network_info.sta_pass, pass_len);
    return err;
}

nw_err_t wifi_network_get_ap_ssid(char* ap_ssid, size_t ssid_len)
{
    nw_err_t err;
    err = secure_string_copy(ap_ssid, (char*)wifi_network_info.ap_ssid, ssid_len);
    return err;
}

nw_err_t wifi_network_get_ap_pass(char* ap_pass, size_t pass_len)
{
    nw_err_t err;
    err = secure_string_copy(ap_pass, (char*)wifi_network_info.ap_pass, pass_len);
    return err;
}

wifi_network_status_e wifi_network_get_state(void)
{
    return wifi_network_state;
}

wifi_network_sta_connection_status_e wifi_network_get_sta_connection_state(void)
{
    return wifi_sta_connection_state;
}

wifi_network_ap_connection_status_e wifi_network_get_ap_connection_state(void)
{
    return wifi_ap_connection_state;
}

int8_t wifi_network_get_rssi(void)
{
    int8_t rssi_data;
    esp_err_t err = esp_wifi_sta_get_rssi(&rssi_data);
    wifi_error_check(err, "func: wifi_network_get_rssi()");
    return rssi_data;
}

//*---------------------------------UTILITY--------------------------------------------//

static err_t secure_string_copy(char *dest, char *src, size_t dest_size)
{
    if(strlen(src) <= dest_size)
    {
        strncpy(dest, src, dest_size);
    }
    else
    {
#if (WIFI_NETWORK_DEBUG_LOG)
        ESP_LOGE("WIFI_NETWORK", "String copy buffer is not enough" );
#endif
        return WIFI_NETWORK_ERROR;
    }
    return WIFI_NETWORK_OK;
}


//*--------------------------------- ERROR CHECK -------------------------------------------- //

//? Eğer programcı hatası söz konusu olursa (yanlış değer girme vs. gibi) assert() kullanmak daha hızlı olacaktır.
static esp_err_t wifi_error_check(esp_err_t err, char *user_msg)
{   

    char error_msg[ERROR_MSG_SIZE] = "\0";

#if (WIFI_NETWORK_CUSTOM_ERROR_CHECK)

    error_check_level_e err_level = NO_ERROR;

    err_to_string(err, error_msg);
    err_level = err_to_level(err);

    switch (err_level)
    {
        case NO_ERROR:
        {
            return ESP_OK;
        }
        break;

        case ERROR_CHECK_LOG_ONLY:
        {
            if(err != ESP_OK)
            {
            ESP_LOGE("WIFI_NETWORK", "%s ERRSTR: %s ERRNO: %d", user_msg, error_msg, err);
            return err;
            }

        }
        break;
        case ERROR_CHECK_LOG_AND_SUSPEND:
        {
            if(err != ESP_OK)
            {
            ESP_LOGE("WIFI_NETWORK", "%s ERRSTR: %s ERRNO: %d", user_msg, error_msg, err);
            vTaskSuspend(NULL);
            return err;
            }
        }
        break;
        case ERROR_CHECK_LOG_AND_ABORT:
        {
            if(err != ESP_OK)
            {
            ESP_LOGE("WIFI_NETWORK", "%s ERRSTR: %s ERRNO: %d", user_msg, error_msg, err);
            abort();
            }
            return err; // :)
        }
        break;
        case ERROR_CHECK_LOG_AND_RAISE_EVENT:
        { // Burada nasıl bir event raise edilebilir? Handler nerede?
            if(err != ESP_OK)
            {
            ESP_LOGE("WIFI_NETWORK", "%s ERRSTR: %s ERRNO: %d", user_msg, error_msg, err);
            // vvvv Bu kısmı sonrasında incele vvvv 
            esp_event_post(WIFI_EVENT, WIFI_EVENT_ERROR, NULL, (size_t)0, portMAX_DELAY);
            }
        }
        break;

    }

#else

    if (err != ESP_OK)
    {
        err_to_string(err, error_msg);
        ESP_LOGE("WIFI_NETWORK", "%s ERRSTR: %s ERRNO: %d", user_msg, error_msg, err)
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
    }

#endif
    
    return ESP_OK;
}

static void err_to_string(esp_err_t err, char * msg) // Bu fonksiyon çalışmazse kendimiz yazacağız
{
    // Burada fonksiyon
#if (!WIFI_NETWORK_CUSTOM_ERROR_CHECK)
    
    if(strlen(esp_err_to_name(err)) <= ERROR_MSG_SIZE)
    {
        strncpy(msg, esp_err_to_name(err), ERROR_MSG_SIZE);
        //esp_err_to_name_r(err, error_msg, ERROR_MSG_SIZE);
    }
    else
    {
        ESP_LOGW("WIFI_NETWORK", "Error message buffer is not enough" );
    }
#else
    switch(err)
    {
        case ESP_FAIL: strncpy(msg, "ESP_FAIL", ERROR_MSG_SIZE); break;
        case ESP_OK: strncpy(msg, "ESP_OK", ERROR_MSG_SIZE); break;
        case ESP_ERR_NO_MEM: strncpy(msg, "ESP_ERR_NO_MEM", ERROR_MSG_SIZE); break;
        case ESP_ERR_INVALID_ARG: strncpy(msg, "ESP_ERR_INVALID_ARG", ERROR_MSG_SIZE); break;                           
        case ESP_ERR_INVALID_STATE: strncpy(msg, "ESP_ERR_INVALID_STATE", ERROR_MSG_SIZE); break;                          
        case ESP_ERR_INVALID_SIZE: strncpy(msg, "ESP_ERR_INVALID_SIZE", ERROR_MSG_SIZE); break;                           
        case ESP_ERR_NOT_FOUND: strncpy(msg, "ESP_ERR_NOT_FOUND", ERROR_MSG_SIZE); break;                              
        case ESP_ERR_NOT_SUPPORTED: strncpy(msg, "ESP_ERR_NOT_SUPPORTED", ERROR_MSG_SIZE); break;                           
        case ESP_ERR_TIMEOUT: strncpy(msg, "ESP_ERR_TIMEOUT", ERROR_MSG_SIZE); break;                                
        case ESP_ERR_INVALID_RESPONSE: strncpy(msg, "ESP_ERR_INVALID_RESPONSE", ERROR_MSG_SIZE); break;                         
        case ESP_ERR_INVALID_CRC: strncpy(msg, "ESP_ERR_INVALID_CRC", ERROR_MSG_SIZE); break;                             
        case ESP_ERR_INVALID_VERSION: strncpy(msg, "ESP_ERR_INVALID_VERSION", ERROR_MSG_SIZE); break;                         
        case ESP_ERR_INVALID_MAC: strncpy(msg, "ESP_ERR_INVALID_MAC", ERROR_MSG_SIZE); break;                            
        case ESP_ERR_NOT_FINISHED: strncpy(msg, "ESP_ERR_NOT_FINISHED", ERROR_MSG_SIZE); break;     
        case WIFI_NETWORK_ERR_WIFI_NOT_INIT: strncpy(msg, "ESP_ERR_WIFI_NOT_INIT", ERROR_MSG_SIZE); break;                  
        case WIFI_NETWORK_ERR_WIFI_NOT_STARTED: strncpy(msg, "ESP_ERR_WIFI_NOT_STARTED", ERROR_MSG_SIZE); break;                
        case WIFI_NETWORK_WIFI_NOT_STOPPED: strncpy(msg, "ESP_ERR_WIFI_NOT_STOPPED", ERROR_MSG_SIZE); break;                  
        case WIFI_NETWORK_ERR_WIFI_IF: strncpy(msg, "ESP_ERR_WIFI_IF", ERROR_MSG_SIZE); break;                        
        case WIFI_NETWORK_ERR_WIFI_MODE: strncpy(msg, "ESP_ERR_WIFI_MODE", ERROR_MSG_SIZE); break;                      
        case WIFI_NETWORK_ERR_WIFI_STATE: strncpy(msg, "ESP_ERR_WIFI_STATE", ERROR_MSG_SIZE); break;                   
        case WIFI_NETWORK_ERR_WIFI_CONN: strncpy(msg, "ESP_ERR_WIFI_CONN", ERROR_MSG_SIZE); break;                     
        case WIFI_NETWORK_ERR_WIFI_NVS: strncpy(msg, "ESP_ERR_WIFI_NVS", ERROR_MSG_SIZE); break;                     
        case WIFI_NETWORK_ERR_WIFI_MAC: strncpy(msg, "ESP_ERR_WIFI_MAC", ERROR_MSG_SIZE); break;                      
        case WIFI_NETWORK_ERR_WIFI_SSID: strncpy(msg, "ESP_ERR_WIFI_SSID", ERROR_MSG_SIZE); break;                     
        case WIFI_NETWORK_ERR_WIFI_PASSWORD: strncpy(msg, "ESP_ERR_WIFI_PASSWORD", ERROR_MSG_SIZE); break;               
        case WIFI_NETWORK_ERR_WIFI_TIMEOUT: strncpy(msg, "ESP_ERR_WIFI_TIMEOUT", ERROR_MSG_SIZE); break;                
        case WIFI_NETWORK_ERR_WIFI_WAKE_FAIL: strncpy(msg, "ESP_ERR_WAKE_FAIL", ERROR_MSG_SIZE); break;                 
        case WIFI_NETWORK_ERR_WIFI_WOULD_BLOCK: strncpy(msg, "ESP_ERR_WOULD_BLOCK", ERROR_MSG_SIZE); break;          
        case WIFI_NETWORK_ERR_WIFI_NOT_CONNECT: strncpy(msg, "ESP_ERR_NOT_CONNECT", ERROR_MSG_SIZE); break;        
        case WIFI_NETWORK_ERR_WIFI_POST: strncpy(msg, "ESP_ERR_WIFI_POST", ERROR_MSG_SIZE); break;                 
        case WIFI_NETWORK_ERR_WIFI_INIT_STATE: strncpy(msg, "ESP_ERR_WIFI_INIT_STATE", ERROR_MSG_SIZE); break;               
        case WIFI_NETWORK_ERR_WIFI_STOP_STATE: strncpy(msg, "ESP_ERR_WIFI_STOP_STATE", ERROR_MSG_SIZE); break;               
        case WIFI_NETWORK_ERR_WIFI_NOT_ASSOC: strncpy(msg, "ESP_ERR_WIFI_NOT_ASSOC", ERROR_MSG_SIZE); break;               
        case WIFI_NETWORK_ERR_WIFI_TX_DISALLOW: strncpy(msg, "ESP_ERR_WIFI_TX_DISALLOW", ERROR_MSG_SIZE); break;              
        case WIFI_NETWORK_ERR_WIFI_REGISTRAR: strncpy(msg, "ESP_ERR_WIFI_REGISTRAR", ERROR_MSG_SIZE); break;            
        case WIFI_NETWORK_ERR_WIFI_WPS_TYPE: strncpy(msg, "ESP_ERR_WPS_TYPE", ERROR_MSG_SIZE); break;           
        case WIFI_NETWORK_ERR_WIFI_WPS_SM: strncpy(msg, "ESP_ERR_WPS_SM", ERROR_MSG_SIZE); break;                     
        case WIFI_NETWORK_ERR_ESP_NETIF_INVALID_PARAMS: strncpy(msg, "ESP_ERR_NETIF_INVALID_PARAMS", ERROR_MSG_SIZE); break;        
        case WIFI_NETWORK_ERR_ESP_NETIF_IF_NOT_READY: strncpy(msg, "ESP_ERR_NETIF_IF_NOT_READY", ERROR_MSG_SIZE); break;          
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCPC_START_FAILED: strncpy(msg, "ESP_ERR_NETIF_DHCPC_START_FAILED", ERROR_MSG_SIZE); break;    
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCP_ALREADY_STARTED: strncpy(msg, "ESP_ERR_NETIF_DHCP_ALREADY_STARTED", ERROR_MSG_SIZE); break;  
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED: strncpy(msg, "ESP_ERR_NETIF_DHCP_ALREADY_STOPPED", ERROR_MSG_SIZE); break;  
        case WIFI_NETWORK_ERR_ESP_NETIF_NO_MEM: strncpy(msg, "ESP_ERR_NETIF_NO_MEM", ERROR_MSG_SIZE); break;                
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCP_NOT_STOPPED: strncpy(msg, "ESP_ERR_NETIF_DHCP_NOT_STOPPED", ERROR_MSG_SIZE); break;      
        case WIFI_NETWORK_ERR_ESP_NETIF_DRIVER_ATTACH_FAILED: strncpy(msg, "ESP_ERR_NETIF_DRIVER_ATTACH_FAILED", ERROR_MSG_SIZE); break;  
        case WIFI_NETWORK_ERR_ESP_NETIF_INIT_FAILED: strncpy(msg, "ESP_ERR_NETIF_INIT_FAILED", ERROR_MSG_SIZE); break;           
        case WIFI_NETWORK_ERR_ESP_NETIF_DNS_NOT_CONFIGURED: strncpy(msg, "ESP_ERR_NETIF_DNS_NOT_CONFIGURED", ERROR_MSG_SIZE); break;    
    }
#endif
}

static error_check_level_e err_to_level(esp_err_t err)
{
    switch (err)  // Bu şekilde ilerleyecek
    {   
        //* Burada hata ele alma seviyeleri aşağıdaki caselere kopyalanarak artırılabilir. (Duruma göre!)
        //? Bu derleyicide switch caseleri içerisinde expression kullanılabiliyor!! (10 + 5 gibi), standart c ve c++ 'ya aykırı
        case ESP_OK: 
        case ESP_ERR_NO_MEM:                
        case ESP_ERR_INVALID_SIZE:               
        case ESP_ERR_NOT_FOUND:                 
        case ESP_ERR_NOT_SUPPORTED:          
        case ESP_ERR_TIMEOUT:   
        case ESP_ERR_INVALID_RESPONSE:       
        case ESP_ERR_INVALID_CRC:         
        case ESP_ERR_INVALID_VERSION:       
        case ESP_ERR_INVALID_MAC:                      
        case ESP_ERR_NOT_FINISHED: 
        case WIFI_NETWORK_ERR_WIFI_NOT_INIT:    
        case WIFI_NETWORK_ERR_WIFI_NOT_STARTED:         
        case WIFI_NETWORK_WIFI_NOT_STOPPED:             
        case WIFI_NETWORK_ERR_WIFI_IF:        
        case WIFI_NETWORK_ERR_WIFI_MODE:   
        case WIFI_NETWORK_ERR_WIFI_STATE:                
        case WIFI_NETWORK_ERR_WIFI_CONN: 
        case WIFI_NETWORK_ERR_WIFI_NVS:                     
        case WIFI_NETWORK_ERR_WIFI_MAC:                       
        case WIFI_NETWORK_ERR_WIFI_SSID:      
        case WIFI_NETWORK_ERR_WIFI_PASSWORD:         
        case WIFI_NETWORK_ERR_WIFI_TIMEOUT:         
        case WIFI_NETWORK_ERR_WIFI_WAKE_FAIL: 
        case WIFI_NETWORK_ERR_WIFI_WOULD_BLOCK: 
        case WIFI_NETWORK_ERR_WIFI_NOT_CONNECT:
        case WIFI_NETWORK_ERR_WIFI_POST: 
        case WIFI_NETWORK_ERR_WIFI_INIT_STATE: 
        case WIFI_NETWORK_ERR_WIFI_STOP_STATE:         
        case WIFI_NETWORK_ERR_WIFI_NOT_ASSOC:        
        case WIFI_NETWORK_ERR_WIFI_TX_DISALLOW:         
        case WIFI_NETWORK_ERR_WIFI_REGISTRAR:        
        case WIFI_NETWORK_ERR_WIFI_WPS_TYPE:      
        case WIFI_NETWORK_ERR_WIFI_WPS_SM:     
        case WIFI_NETWORK_ERR_ESP_NETIF_INVALID_PARAMS: 
        case WIFI_NETWORK_ERR_ESP_NETIF_IF_NOT_READY:  
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCPC_START_FAILED: 
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCP_ALREADY_STARTED: 
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED:
        case WIFI_NETWORK_ERR_ESP_NETIF_NO_MEM: 
        case WIFI_NETWORK_ERR_ESP_NETIF_DHCP_NOT_STOPPED: 
        case WIFI_NETWORK_ERR_ESP_NETIF_DRIVER_ATTACH_FAILED:
        case WIFI_NETWORK_ERR_ESP_NETIF_INIT_FAILED:         
        case WIFI_NETWORK_ERR_ESP_NETIF_DNS_NOT_CONFIGURED:
        {
            return ERROR_CHECK_LOG_ONLY;
        }
        break;

        case ESP_FAIL:
        {
            return ERROR_CHECK_LOG_AND_SUSPEND;
        }
        break;

        case ESP_ERR_INVALID_ARG:
        {
            return ERROR_CHECK_LOG_AND_ABORT;
        }

        case ESP_ERR_INVALID_STATE:
        {
            return ERROR_CHECK_LOG_AND_RAISE_EVENT;
        }
    }

    return ERROR_CHECK_LOG_ONLY;
}


static void reason_to_string(wifi_network_disconnect_reason_e reason, char* reason_string)
{
    switch (reason)
    {
        
    case WIFI_NETWORK_DISCONNECT_REASON_NOT_DEFINED: strncpy(reason_string, "DISCONNECT_REASON_NOT_DEFINED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_UNSPECIFIED: strncpy(reason_string, "DISCONNECT_REASON_UNSPECIFIED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_AUTH_EXPIRE: strncpy(reason_string, "DISCONNECT_REASON_AUTH_EXPIRE", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_AUTH_LEAVE: strncpy(reason_string, "DISCONNECT_REASON_AUTH_LEAVE", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_ASSOC_EXPIRE: strncpy(reason_string, "DISCONNECT_REASON_ASSOC_EXPIRE", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_ASSOC_TOOMANY: strncpy(reason_string, "DISCONNECT_REASON_ASSOC_TOOMANY", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_NOT_AUTHED: strncpy(reason_string, "DISCONNECT_REASON_NOT_AUTHED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_NOT_ASSOCED: strncpy(reason_string, "DISCONNECT_REASON_NOT_ASSOCED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_ASSOC_LEAVE: strncpy(reason_string, "DISCONNECT_REASON_ASSOC_LEAVE", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_ASSOC_NOT_AUTHED: strncpy(reason_string, "DISCONNECT_REASON_ASSOC_NOT_AUTHED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD: strncpy(reason_string, "DISCONNECT_REASON_DISASSOC_PWRCAP_BAD", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD: strncpy(reason_string, "DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_IE_INVALID: strncpy(reason_string, "DISCONNECT_REASON_IE_INVALID", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_MIC_FAILURE: strncpy(reason_string, "DISCONNECT_REASON_MIC_FAILURE", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT: strncpy(reason_string, "DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT: strncpy(reason_string, "DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS: strncpy(reason_string, "DISCONNECT_REASON_IE_IN_4WAY_DIFFERS", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_GROUP_CIPHER_INVALID: strncpy(reason_string, "DISCONNECT_REASON_GROUP_CIPHER_INVALID", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID: strncpy(reason_string, "DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_AKMP_INVALID: strncpy(reason_string, "DISCONNECT_REASON_AKMP_INVALID", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION: strncpy(reason_string, "DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_INVALID_RSN_IE_CAP: strncpy(reason_string, "DISCONNECT_REASON_INVALID_RSN_IE_CAP", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_802_1X_AUTH_FAILED: strncpy(reason_string, "DISCONNECT_REASON_802_1X_AUTH_FAILED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_CIPHER_SUITE_REJECTED: strncpy(reason_string, "DISCONNECT_REASON_CIPHER_SUITE_REJECTED", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_BEACON_TIMEOUT: strncpy(reason_string, "DISCONNECT_REASON_BEACON_TIMEOUT", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_NO_AP_FOUND: strncpy(reason_string, "DISCONNECT_REASON_NO_AP_FOUND", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_AUTH_FAIL: strncpy(reason_string, "DISCONNECT_REASON_AUTH_FAIL", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_ASSOC_FAIL: strncpy(reason_string, "DISCONNECT_REASON_ASSOC_FAIL", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_HANDSHAKE_TIMEOUT: strncpy(reason_string, "DISCONNECT_REASON_HANDSHAKE_TIMEOUT", COMMON_MSG_SIZE); break;
    case WIFI_NETWORK_DISCONNECT_REASON_CONNECTION_FAIL: strncpy(reason_string, "DISCONNECT_REASON_NOT_DEFINED", COMMON_MSG_SIZE); break;

    }
}