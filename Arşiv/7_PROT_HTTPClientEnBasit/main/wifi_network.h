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

typedef int8_t nw_err_t;
// TODO: Bunlar app kütüphanesine taşınacak
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


// GOT_IP bizde ağa bağlı olup olmadığını belirlemek için kullanılacak. İnternete bağlı olup olmadığını bilmek için ping atmak gereklidir.  
// STA ve AP connection status ayrı olacak.
typedef enum
{
    WIFI_NETWORK_STA_STATE_NOTAVAILABLE,
    WIFI_NETWORK_STA_STATE_AVAILABLE,
    WIFI_NETWORK_STA_STATE_DISCONNECTED,
    WIFI_NETWORK_STA_STATE_CONNECTED,
    WIFI_NETWORK_STA_STATE_GOTIP,
    WIFI_NETWORK_STA_STATE_SCANNING
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

typedef enum
{
    NO_ERROR,
    ERROR_CHECK_LOG_ONLY,
    ERROR_CHECK_LOG_AND_SUSPEND,
    ERROR_CHECK_LOG_AND_ABORT,
    ERROR_CHECK_LOG_AND_RAISE_EVENT
} error_check_level_e;
// TODO: Bunun için default config makrosu oluşturulacak



//******************* Bitfields ******************************


//******************* Function Prototypes *********************

// Init/Config Functions

esp_err_t wifi_network_config(wifi_network_info_t *config_info);
esp_err_t wifi_network_init(EventGroupHandle_t event_group);
esp_err_t wifi_network_station_mode(void);
esp_err_t wifi_network_softap_mode(void);
esp_err_t wifi_network_scan_mode(void);
esp_err_t wifi_network_offline(void);
esp_err_t wifi_network_deinit(void);

// Set functions
// Bunlar static yapının değerlerini setleyecek (struct içindeki!!)
nw_err_t wifi_network_set_sta_ssid(char *sta_ssid, size_t ssid_len);
nw_err_t wifi_network_set_sta_pass(char *sta_pass, size_t pass_len);
nw_err_t wifi_network_set_ap_ssid(char *ap_ssid, size_t ssid_len);
nw_err_t wifi_network_set_ap_pass(char *ap_pass, size_t pass_len);
// Get Functions (Bunlar static değerleri döndürecek (struct içindeki!!))
nw_err_t wifi_network_get_sta_ssid(char* sta_ssid, size_t ssid_len);
nw_err_t wifi_network_get_sta_pass(char* sta_pass, size_t pass_len);
nw_err_t wifi_network_get_ap_ssid(char* ap_ssid, size_t ssid_len);
nw_err_t wifi_network_get_ap_pass(char* ap_pass, size_t pass_len);
int8_t wifi_network_get_rssi(void);
wifi_network_status_e wifi_network_get_state(void);
wifi_network_sta_connection_status_e wifi_network_get_sta_connection_state(void);
wifi_network_ap_connection_status_e wifi_network_get_ap_connection_state(void);
// Service Functions

// Get / Set Functions 

#ifdef __cplusplus
}
#endif // extern "C"

#endif // WIFI_NETWORK_H
