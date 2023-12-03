#ifndef WN_HTTP_SERVER_H
#define WN_HTTP_SERVER_H


/**
 * @file 
 * @brief HTTP Server, Captive portal kütüphanesi
 * 
 * 
 * 
 * 
 * 
 * @author Gökhan DÖKMETAŞ
 * @date 27-11-2023
 */
#ifdef __cplusplus
extern "C" {
#endif


//******************** Notes ********************************



//******************** Includes *****************************
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wn_http_server_config.h"
//******************** Features ******************************

//******************** Macros ********************************

//******************** General Constants *********************


#define WN_SERVER_EVENT_ERROR                     BIT0
#define WN_SERVER_EVENT_START                     BIT1
#define WN_SERVER_EVENT_ON_CONNECTED              BIT2
#define WN_SERVER_EVENT_ON_HEADER                 BIT3 
#define WN_SERVER_EVENT_HEADERS_SENT              BIT4
#define WN_SERVER_EVENT_ON_DATA                   BIT5 
#define WN_SERVER_EVENT_SENT_DATA                 BIT6 
#define WN_SERVER_EVENT_DISCONNECTED              BIT7
#define WN_SERVER_EVENT_STOP                      BIT8 
#define WN_SERVER_INTERNAL_EVENT_MASK (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8)

//********************* Globals ******************************


//******************* Value Constants ***********************


//******************* DEFAULT VALUES *************************

//******************* TYPEDEFS *******************************

//******************* Bitfields ******************************


//******************* Function Prototypes *********************

esp_err_t wn_server_stop(void);
esp_err_t wn_server_start(void);
esp_err_t wn_server_init(EventGroupHandle_t wn_server_event_group);

// Event Handlers (External)
void wn_server_sta_connected_handler();
void wn_server_sta_disconnected_handler();
#ifdef __cplusplus
}
#endif // extern "C"

#endif // WN_HTTP_SERVER_H
