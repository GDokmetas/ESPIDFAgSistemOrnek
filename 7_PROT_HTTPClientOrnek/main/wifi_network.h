#ifndef WIFI_NETWORK_H
#define WIFI_NETWORK_H

/**
 * @file wifi_network.h
 * @brief wifi_network kütüphanesinin başlık dosyası
 * 
 * wifi_network.h ESP-IDF Wifi sürücüsünü kullanan ve ağ özelliklerini içeren bir soyutlama katmanıdır. 
 * 
 * 
 * 
 * @author Gökhan DÖKMETAŞ
 * @date 06-11-2023
 */

#ifdef __cplusplus
extern "C" {
#endif


//******************** Notes ********************************
// TODO: State ve status kısmı app kütüphanesinde yer alacak. 
// TODO: State geçişleri bir daha kontrol edilecek. State'lerde bir boşluk olmayacak.
// TODO: Konfigürasyona dair bütün parametreler define'lar içerisine alınacak. (Ayrıntılara yer vermek ölçeklenebilirliği artırır)
// TODO: 

//******************** Includes *****************************
#include "wifi_network_config.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_bit_defs.h"
//******************** Features ******************************


//******************** Macros ********************************

#define WIFI_NETWORK_DEFAULT_CONFIG() { \
    .sta_ssid = WIFI_NETWORK_DEFAULT_STA_SSID, \
    .sta_pass = WIFI_NETWORK_DEFAULT_STA_PASS, \
    .ap_ssid = WIFI_NETWORK_DEFAULT_AP_SSID, \
    .ap_pass = WIFI_NETWORK_DEFAULT_AP_PASS, \
}

// This macro converts ip string to 32 bit unsigned integer 



//******************** General Constants *********************
// Event Bits 
#define WIFI_NETWORK_EVENT_BIT_SCAN_DONE                (BIT0)
#define WIFI_NETWORK_EVENT_BIT_STA_START                (BIT1)
#define WIFI_NETWORK_EVENT_BIT_STA_STOP                 (BIT2)
#define WIFI_NETWORK_EVENT_BIT_STA_CONNECTED            (BIT3)
#define WIFI_NETWORK_EVENT_BIT_STA_DISCONNECTED         (BIT4)
#define WIFI_NETWORK_EVENT_BIT_STA_AUTHMODE_CHANGE      (BIT5)
#define WIFI_NETWORK_EVENT_BIT_AP_START                 (BIT6)
#define WIFI_NETWORK_EVENT_BIT_AP_STOP                  (BIT7)
#define WIFI_NETWORK_EVENT_BIT_AP_STACONNECTED          (BIT8)
#define WIFI_NETWORK_EVENT_BIT_AP_STADISCONNECTED       (BIT9)
#define WIFI_NETWORK_EVENT_BIT_AP_PROBEREQRECVED        (BIT10)
#define WIFI_NETWORK_EVENT_BIT_STA_BSS_RSSI_LOW         (BIT11)
#define WIFI_NETWORK_EVENT_BIT_STA_BEACON_TIMEOUT       (BIT12)
#define WIFI_NETWORK_EVENT_BIT_STA_GOT_IP               (BIT14)
#define WIFI_NETWORK_EVENT_BIT_STA_LOST_IP              (BIT15)
#define WIFI_NETWORK_EVENT_BIT_AP_STAIPASSIGNED         (BIT16)

#define WIFI_NETWORK_EVENT_BIT_EVENT_ERROR              (BIT30)
#define WIFI_NETWORK_EVENT_BIT_UNKNOWN_EVENT            (BIT31)

//********************* Globals ******************************
//******************* TYPEDEFS *******************************


// Basic Types 
typedef int8_t nw_err_t;

// Enumerations
typedef enum 
{
    WIFI_NETWORK_STATE_DEINIT = 0,
    WIFI_NETWORK_STATE_INIT,
    WIFI_NETWORK_STATE_STA,
    WIFI_NETWORK_STATE_AP,
    WIFI_NETWORK_STATE_SCAN,
    WIFI_NETWORK_STATE_OFFLINE,
    WIFI_NETWORK_STATUS_UNDEFINED = 99
} wifi_network_status_e; 

typedef enum
{
    WIFI_NETWORK_STA_STATE_NOTAVAILABLE,
    WIFI_NETWORK_STA_STATE_AVAILABLE,
    WIFI_NETWORK_STA_STATE_DISCONNECTED,
    WIFI_NETWORK_STA_STATE_CONNECTED,
    WIFI_NETWORK_STA_STATE_GOTIP,
    WIFI_NETWORK_STA_STATE_SCANNING // Scan state bağlı iken olsa da biz ayrı bir state olarak ele alacağız.
} wifi_network_sta_connection_status_e; 

typedef enum 
{
    WIFI_NETWORK_AP_STATE_NOTAVAILABLE,
    WIFI_NETWORK_AP_STATE_AVAILABLE,
    WIFI_NETWORK_AP_STATE_CLIENT_CONNECTED,
    WIFI_NETWORK_AP_STATE_CLIENT_DISCONNECTED,
    WIFI_NETWORK_AP_STATE_CLIENT_GOT_IP
} wifi_network_ap_connection_status_e;

typedef enum
{
    WIFI_NETWORK_PROCESS_WAITING = 0,
    WIFI_NETWORK_PROCESS_SCAN_DONE,
    WIFI_NETWORK_PROCESS_WRONG_PASSWORD,
    WIFI_NETWORK_PROCESS_AP_CLIENT_CONNECTED,
    WIFI_NETWORK_PROCESS_AP_CLIENT_DISCONNECTED
} wifi_network_process_status_e; 

typedef enum
{
    WIFI_NETWORK_DISCONNECT_REASON_NOT_DEFINED = 0,
    WIFI_NETWORK_DISCONNECT_REASON_UNSPECIFIED = 1,
    WIFI_NETWORK_DISCONNECT_REASON_AUTH_EXPIRE = 2,
    WIFI_NETWORK_DISCONNECT_REASON_AUTH_LEAVE = 3,
    WIFI_NETWORK_DISCONNECT_REASON_ASSOC_EXPIRE = 4,
    WIFI_NETWORK_DISCONNECT_REASON_ASSOC_TOOMANY = 5,
    WIFI_NETWORK_DISCONNECT_REASON_NOT_AUTHED = 6,
    WIFI_NETWORK_DISCONNECT_REASON_NOT_ASSOCED = 7,
    WIFI_NETWORK_DISCONNECT_REASON_ASSOC_LEAVE = 8,
    WIFI_NETWORK_DISCONNECT_REASON_ASSOC_NOT_AUTHED = 9,
    WIFI_NETWORK_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD = 10,
    WIFI_NETWORK_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD = 11,
    WIFI_NETWORK_DISCONNECT_REASON_IE_INVALID = 13,
    WIFI_NETWORK_DISCONNECT_REASON_MIC_FAILURE = 14,
    WIFI_NETWORK_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT = 15,
    WIFI_NETWORK_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,
    WIFI_NETWORK_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS = 17,
    WIFI_NETWORK_DISCONNECT_REASON_GROUP_CIPHER_INVALID = 18,
    WIFI_NETWORK_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID = 19,
    WIFI_NETWORK_DISCONNECT_REASON_AKMP_INVALID = 20,
    WIFI_NETWORK_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION = 21,
    WIFI_NETWORK_DISCONNECT_REASON_INVALID_RSN_IE_CAP = 22,
    WIFI_NETWORK_DISCONNECT_REASON_802_1X_AUTH_FAILED = 23,
    WIFI_NETWORK_DISCONNECT_REASON_CIPHER_SUITE_REJECTED = 24,
    WIFI_NETWORK_DISCONNECT_REASON_BEACON_TIMEOUT = 200,
    WIFI_NETWORK_DISCONNECT_REASON_NO_AP_FOUND = 201,
    WIFI_NETWORK_DISCONNECT_REASON_AUTH_FAIL = 202,
    WIFI_NETWORK_DISCONNECT_REASON_ASSOC_FAIL = 203,
    WIFI_NETWORK_DISCONNECT_REASON_HANDSHAKE_TIMEOUT = 204,
    WIFI_NETWORK_DISCONNECT_REASON_CONNECTION_FAIL = 205
} wifi_network_disconnect_reason_e;

typedef enum
{
    NO_ERROR,
    ERROR_CHECK_LOG_ONLY,
    ERROR_CHECK_LOG_AND_SUSPEND,
    ERROR_CHECK_LOG_AND_ABORT,
    ERROR_CHECK_LOG_AND_RAISE_EVENT
} error_check_level_e;

// Structures

typedef struct 
{
    uint8_t sta_ssid[WIFI_NETWORK_MAX_SSID_LEN];
    uint8_t sta_pass[WIFI_NETWORK_MAX_PASS_LEN];
    uint8_t ap_ssid[WIFI_NETWORK_MAX_SSID_LEN];
    uint8_t ap_pass[WIFI_NETWORK_MAX_PASS_LEN];
} wifi_network_info_t;

typedef struct
{
    wifi_network_info_t network_info; 
    int8_t status;
} wifi_network_config_t;


typedef struct 
{
    bool is_new_sta; // Yeni istemci bağlandı mı? Bunu uygulama tarafında false yapacağız.
    uint8_t num_connected_sta; // Bağlı istemci sayısı
    uint8_t channel; // AP'nin kanalı
    uint8_t last_sta_mac[6]; // Son bağlanan istemcinin mac adresi
    uint8_t last_sta_aid; // Son bağlanan istemciye verilen aid 
    uint32_t last_sta_ip; // Son bağlanan istemcinin IP adresi
} wifi_network_ap_stats_t;

typedef struct
{
    uint8_t ssid[WIFI_NETWORK_MAX_SSID_LEN]; // Bağlı bulunan AP'nin SSID'si
    uint8_t mac[6]; // Bağlı bulunan aygıtın MAC Adresi
    uint8_t bssid[6]; // Bağlı bulunan AP'nin MAC Adresi  
    uint8_t channel; // Bağlı bulunan AP'nin kanalı
    wifi_auth_mode_t authmode; // Bağlı bulunan AP'nin doğrulama modu 
    uint16_t aid; // Bağlı bulunan AP'nin doğrulama kimlik değeri
    uint32_t ip; // Bağlanan aygıtın IP adresi
    uint32_t netmask; // Bağlanan aygıtın ağ geçidi
    uint32_t gateway; // Bağlanan aygıtın ağ maskesi
} wifi_network_sta_stats_t;

typedef struct 
{
    bool enabled;
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
    uint32_t dns1;
    uint32_t dns2;
} wifi_network_sta_static_ip_conf_t;


//******************* Bitfields ******************************


//******************* Function Prototypes *********************

// Init/Config Functions

esp_err_t wifi_network_config(wifi_network_info_t *config_info);
esp_err_t wifi_network_config_static_ip(wifi_network_sta_static_ip_conf_t *static_ip_conf);

esp_err_t wifi_network_init(EventGroupHandle_t event_group);
esp_err_t wifi_network_station_mode(void);
esp_err_t wifi_network_softap_mode(void);
esp_err_t wifi_network_scan_mode(void);
esp_err_t wifi_network_offline(void);
esp_err_t wifi_network_deinit(void);

// Set functions
nw_err_t wifi_network_set_sta_ssid(char *sta_ssid, size_t ssid_len);
nw_err_t wifi_network_set_sta_pass(char *sta_pass, size_t pass_len);
nw_err_t wifi_network_set_ap_ssid(char *ap_ssid, size_t ssid_len);
nw_err_t wifi_network_set_ap_pass(char *ap_pass, size_t pass_len);
// Get Functions 
nw_err_t wifi_network_get_sta_ssid(char* sta_ssid, size_t ssid_len);
nw_err_t wifi_network_get_sta_pass(char* sta_pass, size_t pass_len);
nw_err_t wifi_network_get_ap_ssid(char* ap_ssid, size_t ssid_len);
nw_err_t wifi_network_get_ap_pass(char* ap_pass, size_t pass_len);
int16_t wifi_network_get_rssi(void);
wifi_network_status_e wifi_network_get_state(void);
wifi_network_sta_connection_status_e wifi_network_get_sta_connection_state(void);
wifi_network_ap_connection_status_e wifi_network_get_ap_connection_state(void);
wifi_network_disconnect_reason_e wifi_network_get_sta_disconnect_reason(void);
wifi_network_disconnect_reason_e wifi_network_get_ap_disconnect_reason(void);

esp_err_t wifi_network_get_sta_stats(wifi_network_sta_stats_t *sta_stats);
esp_err_t wifi_network_get_ap_stats(wifi_network_ap_stats_t *ap_stats);



// Utility Functions
nw_err_t wifi_network_convert_str_ip_u32(char *ipaddr, uint32_t *addr);
nw_err_t wifi_network_generate_static_ip_conf(wifi_network_sta_static_ip_conf_t *ip_config, char* ip, char* netmask, char* gateway, char* dns1, char* dns2);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // WIFI_NETWORK_H
