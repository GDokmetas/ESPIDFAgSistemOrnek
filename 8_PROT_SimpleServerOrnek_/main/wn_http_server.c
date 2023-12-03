// Author: Gökhan DÖKMETAŞ

//********************Includes********************************
#include "wn_http_server.h"
#include "wn_nvs.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/task.h"
#include "esp_macros.h"
#include <string.h>
#include <ctype.h>
//******************** Notes *********************************
// TODO: NVS'De kayıtlı veriler web sayfasında gösterilebilir. 
// TODO: Web sayfası dinamik olabilir.
//*********************Private Data Types ********************

//******************** Private Globals ***********************
static httpd_handle_t http_server = NULL;
static EventGroupHandle_t _server_event_group = NULL;

//******************** Private Defines ***********************
#define MAX_NVS_VALUE_LEN (64)
#define ERR_STATE_STR_SIZE (500)
#define RESPONSE_HTML_HEAD "<html><head><meta charset=\"utf-8\"><title>Impeklub</title></head><body>"
#define RESPONSE_HTML_TRAIL "</body></html>"

// NVS Parametre Hata durumları
#define NVS_PARAM_OK                            (0)
#define NVS_PARAM_KEY_ERROR                     (BIT1)
#define NVS_PARAM_EMPTY_VALUE                   (BIT2)
#define NVS_PARAM_SSID_LEN_SHORT                (BIT3)
#define NVS_PARAM_SSID_LEN_LONG                 (BIT4)
#define NVS_PARAM_PASS_LEN_SHORT                (BIT5)
#define NVS_PARAM_NON_NUMERIC_VALUE             (BIT6) 
#define NVS_PARAM_NON_ALPHANUMERIC_VALUE        (BIT7)
#define NVS_PARAM_NON_CHARACTER_VALUE           (BIT8)
#define NVS_PARAM_NON_PRINTABLE_VALUE           (BIT9)
#define NVS_PARAM_MQTT_PORT_LEN_LONG            (BIT10)

#define NVS_PARAM_KEY_ERROR_STR_TR              "HATA!: Parametre Anahtarı Hatalı<br>"
#define NVS_PARAM_EMPTY_VALUE_STR_TR            "HATA!: Parametreler Boş Bırakılamaz<br>"
#define NVS_PARAM_SSID_LEN_SHORT_STR_TR         "HATA!: Ağ Adı En Az 2 Karakter Olmalı<br>"
#define NVS_PARAM_SSID_LEN_LONG_STR_TR          "HATA!: Ağ Adı En Fazla 32 Karakter Olmalı<br>"
#define NVS_PARAM_PASS_LEN_SHORT_STR_TR         "HATA!: Şifre En Az 8 Karakter Olmalı<br>"
#define NVS_PARAM_NON_NUMERIC_VALUE_STR_TR      "HATA!: Sayısal Olmayan Değer Girilemez<br>"
#define NVS_PARAM_NON_ALPHANUMERIC_VALUE_STR_TR "HATA!: Alfanümerik Olmayan Değer Girilemez<br>"
#define NVS_PARAM_NON_CHARACTER_VALUE_STR_TR    "HATA!: Karakter Olmayan Değer Girilemez<br>"
#define NVS_PARAM_NON_PRINTABLE_VALUE_STR_TR    "HATA!: Yazdırılabilir Olmayan Değer Girilemez<br>"
#define NVS_PARAM_MQTT_PORT_LEN_LONG_STR_TR     "HATA!: Port Numarası 5 Karakterden Fazla Olamaz<br>"
// strncat'de eğer bufferde yeterli alan yoksa undefined behaivor gerçekleşmektedir. 
//* Derleyici bunun hakkında önceden uyarı verse de işi sağlama almak gereklidir!


// Parametre hata durumları (Bit olarak)
//******************** Private Function Prototypes ***********
static esp_err_t http_server_root_get_handler(httpd_req_t *req);
static esp_err_t http_server_form_post_handler(httpd_req_t *req);
static esp_err_t http_server_error_handler(httpd_req_t *req, httpd_err_code_t error);
static void http_server_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wn_server_sta_connected_handler();
void wn_server_sta_disconnected_handler();
static uint32_t wn_search_and_save_nvs(const char* key, const char* value);

//******************** Private Constants *********************
static const httpd_uri_t http_server_root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = http_server_root_get_handler,
    .user_ctx = NULL
};

static const httpd_uri_t http_server_form_post_uri = {
    .uri = "/setting",
    .method = HTTP_POST,
    .handler = http_server_form_post_handler,
    .user_ctx = NULL
};


//! Burada web sayfasını const char* olarak tanımlayacağız! Bunu SPIFFS'de dosya olarak tanımlamak ve parçalı göndermek
//!gerekebilir. Şimdilik idare etmesi için böyle yapılıyor.

const char *wn_server_form_page = "<!DOCTYPE html>" 
"<html lang=\"tr\">"
"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\">"
"<title>Config ESP</title>"
"<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();};" 
"function pw(id) { var x=document.getElementById(id); if(x.type==='password') {x.type='text';}"
"else {x.type='password';} };</script>"
"<style>"
".de{background-color:#ffaaaa;} .em{font-size:0.8em;color:#bb0000;padding-bottom:0px;}"
".c{text-align: center;} div,input,select{padding:5px;font-size:1em;} input{width:95%;} select{width:100%} "
"input[type=checkbox]{width:auto;scale:1.5;margin:10px;} body{text-align: center;font-family:verdana;}"
"button{border:0;border-radius:0.3rem;background-color:#16A1E7;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} fieldset{border-radius:0.3rem;margin: 0px;}"
"</style>"
"</head>"
"<body><div style=\"text-align:left;display:inline-block;min-width:260px;\">"
"<form action=\"http://192.168.4.1/setting\" method=\"post\"><input type=\"hidden\" name=\"iotSave\" value=\"true\">"
"<fieldset id=\"system_config\"><legend>Sistem Ayarları</legend>"
"<div class=""><label for=\"ap_name\">AP Ağ Adı</label><input type=\"text\" id=\"ap_name\" name=\"ap_name\" 32=\"\" placeholder=\"\" value=\"\"><div class=\"em\"></div></div>"
"<div class=""><label for=\"ap_pass\">AP Şifresi</label><input type=\"password\" id=\"ap_pass\" name=\"ap_pass\" 32=\"\" placeholder=\"\" value=\"\" ondblclick=\"pw(this.id)\"><div class=\"em\"></div></div>"
"<div class=\"\"><label for=\"sta_ssid\">WiFi Ağ Adı</label><input type=\"text\" id=\"sta_ssid\" name=\"sta_ssid\" 32=\"\" placeholder=\"\" value=\"\"><div class=\"em\"></div></div>"
"<div class=\"\"><label for=\"sta_pass\">WiFi Şifresi</label><input type=\"password\" id=\"sta_pass\" name=\"sta_pass\" 32=\"\" placeholder=\"\" value=\"\" ondblclick=\"pw(this.id)\"><div class=\"em\"></div></div>"
"</fieldset>"
#if (WN_SERVER_MQTT_CONFIG_ENABLED)
"<fieldset id=\"mqtt_config\"><legend>MQTT Ayarları</legend>"
"<div class=\"\"><label for=\"mqqt_name\">Sunucu</label><input type=\"text\" id=\"mqtt_name\" name=\"mqtt_name\" 79=\"\" placeholder=\"\" value=\"\"><div class=\"em\"></div></div>"
"<div class=\"\"><label for=\"mqtt_port\">Port</label><input type=\"text\" id=\"mqtt_port\" name=\"mqtt_port\" 5=\"\" placeholder=\"\" value=\"\"><div class=\"em\"></div></div>"
"<div class=\"\"><label for=\"mqtt_user\">Kullanıcı Adı</label><input type=\"text\" id=\"mqtt_user\" name=\"mqtt_user\" 34=\"\" placeholder=\"\" value=\"\"><div class=\"em\"></div></div>"
"<div class=\"\"><label for=\"mqtt_pass\">MQTT Şifre</label><input type=\"text\" id=\"mqtt_pass\" name=\"mqtt_pass\" 34=\"\" placeholder=\"\" value=\"\"><div class=\"em\"></div></div>"
"</fieldset>"
#endif
"<button type=\"submit\" style=\"margin-top: 10px;\">Kaydet</button></form>"
"</div></body></html>";
//******************** Private Macros ************************
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

//------------------ Method/URI Handlers -----------------------

static esp_err_t http_server_root_get_handler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("HTTP SERVER", "HTTP SERVER ROOT GET HANDLER");
    #endif
    httpd_resp_send(req, wn_server_form_page, HTTPD_RESP_USE_STRLEN);
    return err;
}

static esp_err_t http_server_form_post_handler(httpd_req_t *req)
{
    esp_err_t err = ESP_OK;
    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("HTTP SERVER", "HTTP SERVER FORM POST HANDLER");
    #endif

        /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[500];
    memset(content, 0, sizeof(content));

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /*
    for(int i = 0; i < sizeof(content); i++)
    {
        printf("%02x ", (unsigned char)content[i]);
    }
    */
    //ESP_LOGW("HTTP SERVER", "HTTP SERVER FORM POST HANDLER CONTENT: %s", content);
    /* Send a simple response */
    //httpd_resp_sendstr_chunk(req, RESPONSE_HTML_HEAD "<h1>Veri Gönderildi...</h1>" RESPONSE_HTML_TRAIL);

    // Bu noktada parse işlemi yapılmalı!. Sadece veriler NVS'e kaydedilecek. Sonrasında okuma işi yapılacak
    // Veriler okunacak!, eğer düzgün veri varsa kayıt yapılacak. Boş kısım varsa hata verecek.
    // ap_name -> AP NAME
    // ap_pass -> AP PASS
    // sta_ssid -> STA SSID
    // sta_pass -> STA PASS
    // mqtt_name -> MQTT SERVER NAME
    // mqtt_port -> MQTT SERVER PORT
    // mqtt_user -> MQTT SERVER USER
    // mqtt_pass -> MQTT SERVER PASS
    enum {KEY, VALUE} token_state = KEY;
    char* data_ptr = content;
    char* data_ptr_end = content + req->content_len;
    // Burada alacağımız veriler belli! Hiç dinamik yapılara girmeye gerek yok.
    char tmp_key[MAX_NVS_VALUE_LEN];
    char tmp_value[MAX_NVS_VALUE_LEN];
    memset(tmp_key, 0, MAX_NVS_VALUE_LEN);
    memset(tmp_value, 0, MAX_NVS_VALUE_LEN);
    int8_t value_counter = 0;
    uint32_t err_state = 0;
    while (data_ptr != data_ptr_end)
    {
        if(*data_ptr == '&')
        {
            value_counter = 0;
            token_state = KEY;
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("TOKEN", "Key: %s Value: %s", tmp_key, tmp_value);
            #endif
            err_state |= wn_search_and_save_nvs(tmp_key, tmp_value);
            memset(tmp_key, 0, MAX_NVS_VALUE_LEN);
            memset(tmp_value, 0, MAX_NVS_VALUE_LEN);        
        }
        else if(*data_ptr == '=')
        {
            token_state = VALUE;
            value_counter = 0;
        }
        else if(*data_ptr == 0)
        {
            break;
        }
        else
        {
            if(token_state == KEY)
            {
                *(tmp_key + value_counter) = *data_ptr;
            }
            else if(token_state == VALUE)
            {
                *(tmp_value + value_counter) = *data_ptr;
            }
            value_counter++;
            if(value_counter == MAX_NVS_VALUE_LEN) // Hafızayı patlatmamak için!
            {
                break;
            }
        }
        data_ptr++;

    }
    err_state |= wn_search_and_save_nvs(tmp_key, tmp_value);
    value_counter = 0;
    token_state = KEY;
    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("TOKEN", "Key: %s Value: %s", tmp_key, tmp_value);
    #endif
    //wn_search_and_save_nvs(tmp_key, tmp_value);
    //memset(tmp_key, 0, MAX_NVS_VALUE_LEN);
    //memset(tmp_value, 0, MAX_NVS_VALUE_LEN);  

    // Güvenlik önlemleri:
    // strncmp ile key değerleri tek tek kontrol edilecek
    // verinin olup olmadığı kontrol edilecek
    // Verinin istenilen biçimde olup olmadığı kontrol edilecek

    #ifdef WN_SERVER_FORM_PARAM_CHECK_ENABLED

    char error_state[ERR_STATE_STR_SIZE];
    memset(error_state, 0, ERR_STATE_STR_SIZE);
    strncat(error_state, RESPONSE_HTML_HEAD, ERR_STATE_STR_SIZE - 1);
    strncat(error_state, "<h1 style=\"color:red;\">", ERR_STATE_STR_SIZE - 1);
    uint16_t total_written_size = 0;
    if(err_state) // Buradaki hatalar genel hatalar 
    {
        while(err_state)
        {
            if(err_state & NVS_PARAM_KEY_ERROR)
            {
                // Bütün boyutun yanında + \0 karakteri için de bir yer ayrılmalıdır. 
                if (total_written_size + strlen(NVS_PARAM_KEY_ERROR_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_KEY_ERROR_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_KEY_ERROR_STR_TR);
                err_state &= ~NVS_PARAM_KEY_ERROR;
            }
                

            else if(err_state & NVS_PARAM_EMPTY_VALUE)
            {
                if (total_written_size + strlen(NVS_PARAM_EMPTY_VALUE_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_EMPTY_VALUE_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_EMPTY_VALUE_STR_TR);
                err_state &= ~NVS_PARAM_EMPTY_VALUE;
            }
              

            else if (err_state & NVS_PARAM_SSID_LEN_SHORT)
            {
                if (total_written_size + strlen(NVS_PARAM_SSID_LEN_SHORT_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_SSID_LEN_SHORT_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_SSID_LEN_SHORT_STR_TR);
                err_state &= ~NVS_PARAM_SSID_LEN_SHORT;
            }
              

            else if (err_state & NVS_PARAM_SSID_LEN_LONG)
            {
                if (total_written_size + strlen(NVS_PARAM_SSID_LEN_LONG_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_SSID_LEN_LONG_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_SSID_LEN_LONG_STR_TR);
                err_state &= ~NVS_PARAM_SSID_LEN_LONG;
            }
              
            else if (err_state & NVS_PARAM_PASS_LEN_SHORT)
            {
                if (total_written_size + strlen(NVS_PARAM_PASS_LEN_SHORT_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_PASS_LEN_SHORT_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_PASS_LEN_SHORT_STR_TR);
                err_state &= ~NVS_PARAM_PASS_LEN_SHORT;
            }

            else if (err_state & NVS_PARAM_NON_NUMERIC_VALUE)
            {
                if (total_written_size + strlen(NVS_PARAM_NON_NUMERIC_VALUE_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_NON_NUMERIC_VALUE_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_NON_NUMERIC_VALUE_STR_TR);
                err_state &= ~NVS_PARAM_NON_NUMERIC_VALUE;
            }
                

            else if (err_state & NVS_PARAM_NON_ALPHANUMERIC_VALUE)
            {
                if (total_written_size + strlen(NVS_PARAM_NON_ALPHANUMERIC_VALUE_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_NON_ALPHANUMERIC_VALUE_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_NON_ALPHANUMERIC_VALUE_STR_TR);
                err_state &= ~NVS_PARAM_NON_ALPHANUMERIC_VALUE;
            }
                

            else if (err_state & NVS_PARAM_NON_CHARACTER_VALUE)
            {
                if (total_written_size + strlen(NVS_PARAM_NON_CHARACTER_VALUE_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_NON_CHARACTER_VALUE_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_NON_CHARACTER_VALUE_STR_TR);
                err_state &= ~NVS_PARAM_NON_CHARACTER_VALUE;
            }
                

            else if (err_state & NVS_PARAM_NON_PRINTABLE_VALUE)
            {
                if (total_written_size + strlen(NVS_PARAM_NON_PRINTABLE_VALUE_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_NON_PRINTABLE_VALUE_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_NON_PRINTABLE_VALUE_STR_TR);
                err_state &= ~NVS_PARAM_NON_PRINTABLE_VALUE;
            }

            #if (WN_SERVER_MQTT_CONFIG_ENABLED)

            else if (err_state & NVS_PARAM_MQTT_PORT_LEN_LONG)
            {
                if (total_written_size + strlen(NVS_PARAM_MQTT_PORT_LEN_LONG_STR_TR) + 1 > ERR_STATE_STR_SIZE)
                {
                    break;
                }
                strncat(error_state, NVS_PARAM_MQTT_PORT_LEN_LONG_STR_TR, ERR_STATE_STR_SIZE - 1);
                total_written_size += strlen(NVS_PARAM_MQTT_PORT_LEN_LONG_STR_TR);
                err_state &= ~NVS_PARAM_MQTT_PORT_LEN_LONG;
            }

            #endif
            
            else // tanımlanmamış hata varsa döngüden çık!
            {
                err_state = 0;
            }
            
        } // while (err_state)
    }
    else
    {
        strncat(error_state, "Kayıt Başarılı<br>", ERR_STATE_STR_SIZE - 1);
    }
    strncat(error_state, "</h1>", ERR_STATE_STR_SIZE - 1);
    strncat(error_state, RESPONSE_HTML_TRAIL, ERR_STATE_STR_SIZE - 1);

    // Bu kısımda hata durumuna göre mesaj gönderilecek.
    httpd_resp_sendstr_chunk(req, RESPONSE_HTML_HEAD);
    httpd_resp_sendstr_chunk(req, error_state);
    httpd_resp_sendstr_chunk(req, RESPONSE_HTML_TRAIL);

    #else

    httpd_resp_send(req, RESPONSE_HTML_HEAD "<h1>Kayıt Başarılı</h1>" RESPONSE_HTML_TRAIL, HTTPD_RESP_USE_STRLEN);

    #endif // FORM_PARAM_CHECK_ENABLED

    return err;
}

//--------------------- Error Handlers --------------------------


static esp_err_t http_server_error_handler(httpd_req_t *req, httpd_err_code_t error)
{
    esp_err_t err = ESP_OK;
    switch (error)
    {
    case HTTPD_500_INTERNAL_SERVER_ERROR:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal Server Error");
        #else
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal Server Error");
        #endif
    }
    break;
    
    case HTTPD_501_METHOD_NOT_IMPLEMENTED:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_501_METHOD_NOT_IMPLEMENTED, "Method Not Implemented");
        #else
        httpd_resp_send_err(req, HTTPD_501_METHOD_NOT_IMPLEMENTED, "Method Not Implemented");
        #endif
    }
    break;
    
    case HTTPD_505_VERSION_NOT_SUPPORTED:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_505_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
        #else
        httpd_resp_send_err(req, HTTPD_505_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
        #endif
    }
    break;
    
    case HTTPD_400_BAD_REQUEST:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Request");
        #else
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Request");
        #endif
    }
    break;

    case HTTPD_401_UNAUTHORIZED:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
        #else
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized");
        #endif
    }
    break;

    case HTTPD_403_FORBIDDEN:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Forbidden");
        #else
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Forbidden");
        #endif
    }
    break;

    case HTTPD_404_NOT_FOUND:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not Found");
        #else
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not Found");
        #endif
    }
    break;

    case HTTPD_405_METHOD_NOT_ALLOWED:
    {   
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Method Not Allowed");
        #else
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Method Not Allowed");
        #endif    
    }
    break;

    case HTTPD_408_REQ_TIMEOUT:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_408_REQ_TIMEOUT, "Request Timeout");
        #else
        httpd_resp_send_err(req, HTTPD_408_REQ_TIMEOUT, "Request Timeout");
        #endif
    }
    break;

    case HTTPD_411_LENGTH_REQUIRED:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_411_LENGTH_REQUIRED, "Length Required");
        #else
        httpd_resp_send_err(req, HTTPD_411_LENGTH_REQUIRED, "Length Required");
        #endif
    }
    break;

    case HTTPD_414_URI_TOO_LONG:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_414_URI_TOO_LONG, "URIevent_handlerHTTP Too Long");
        #else
        httpd_resp_send_err(req, HTTPD_414_URI_TOO_LONG, "URIevent_handlerHTTP Too Long");
        #endif
    }
    break;

    case HTTPD_431_REQ_HDR_FIELDS_TOO_LARGE:
    {
        #if (WN_SERVER_CUSTOM_ERROR_HTML)
        httpd_resp_send_err(req, HTTPD_431_REQ_HDR_FIELDS_TOO_LARGE, "Request Header Fields Too Large");
        #else
        httpd_resp_send_err(req, HTTPD_431_REQ_HDR_FIELDS_TOO_LARGE, "Request Header Fields Too Large");
        #endif
    }
    break;

    default:
    break;

    }

    return ESP_OK;
}

//--------------------- Event Handlers -----------------------------

static void http_server_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    esp_http_server_event_id_t server_event_id = (esp_http_server_event_id_t)event_id;

    switch (server_event_id)
    {
        case HTTP_SERVER_EVENT_ERROR: // Çalışma sürecinde herhangi bir hata gerçekleştiğinde 
        {
            httpd_err_code_t server_error_code = *((httpd_err_code_t*)event_data);
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_ERROR %s", esp_err_to_name(server_error_code));
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_ERROR);
        }
        break;

        case HTTP_SERVER_EVENT_START: // HTTP sunucusu başladığında gerçekleşir. 
        {
            //Data NULL 
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_START");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_START);
        }
        break;

        case HTTP_SERVER_EVENT_ON_CONNECTED: // HTTP sunucusu istemciye bağlandığında, henüz veri alışverişi başlamadığında gerçekleşir. 
        {
            int socket_desc = *((int*)event_data); // accept() metodundan geri dönen değer
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_ON_CONNECTED");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_ON_CONNECTED);
        }
        break;

        case HTTP_SERVER_EVENT_ON_HEADER: // İstemciden her başlık geldiğinde gerçekleşir
        {
            int socket_desc = *((int*)event_data); // socket descriptor
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_ON_HEADER");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_ON_HEADER);
        }
        break;

        case HTTP_SERVER_EVENT_HEADERS_SENT: // İstemciye bütün başlıklar gönderildi
        {
            int socket_desc = *((int*)event_data); // socket descriptor
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_HEADERS_SENT");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_HEADERS_SENT);
        }
        break;

        case HTTP_SERVER_EVENT_ON_DATA: // İstemciden veri alındı/alınmakta
        {
            esp_http_server_event_data* http_server_event_data = (esp_http_server_event_data*)event_data;
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_ON_DATA");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_ON_DATA);
        }
        break;

        case HTTP_SERVER_EVENT_SENT_DATA: // Session finished
        {
            esp_http_server_event_data* http_server_event_data = (esp_http_server_event_data*)event_data;
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_SENT_DATA");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_SENT_DATA);
        }
        break;

        case HTTP_SERVER_EVENT_DISCONNECTED: // Bağlantı koptu
        {
            int socket_desc = *((int*)event_data); // accept() metodundan geri dönen değer
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_DISCONNECTED");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_DISCONNECTED);
        }
        break;

        case HTTP_SERVER_EVENT_STOP: // Sunucu durduruldu
        {
            // Data NULL
            #if (WN_SERVER_LOG_ENABLED)
            ESP_LOGI("WN HTTP SERVER", "HTTP_SERVER_EVENT_STOP");
            #endif
            xEventGroupSetBits(_server_event_group, WN_SERVER_EVENT_STOP);
        }
        break;
    }
}

//-------------------- WIFI NETWORK EVENT HANDLERS --------------------
// WIFI NETWORK >-->----> WIFI APP ->----->------> HTTP SERVER  
// TODO: fonksiyon isimleri düzenle, check event fonksiyonu koy. 
void wn_server_sta_connected_handler()
{
    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("WN HTTP SERVER", "STA CONNECTED EXT HANDLER");
    #endif
}

void wn_server_sta_disconnected_handler()
{
    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("WN HTTP SERVER", "STA DISCONNECTED EXT HANDLER");
    #endif
}

//********************* Public Functions**********************

esp_err_t wn_server_init(EventGroupHandle_t wn_server_event_group)
{
    _server_event_group = wn_server_event_group;
    esp_event_handler_register(ESP_HTTP_SERVER_EVENT, ESP_EVENT_ANY_ID, &http_server_event_handler, NULL);   
    return ESP_OK;
}


esp_err_t wn_server_start(void)
{
    esp_err_t err = ESP_OK;
    httpd_config_t http_server_config = HTTPD_DEFAULT_CONFIG();
    http_server_config.lru_purge_enable = true;
    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("WN HTTP SERVER", "HTTP SERVER START on port: %d", http_server_config.server_port);
    #endif
    err = httpd_start(&http_server, &http_server_config);
    if(err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER START ERROR: %s", esp_err_to_name(err));
        #endif
        return err;
    }

    //* Error handler kayıt, her birinde ayrı ayrı fonk aldığı için tek tek çağırmalıyız.

    err = httpd_register_err_handler(http_server, HTTPD_500_INTERNAL_SERVER_ERROR, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_501_METHOD_NOT_IMPLEMENTED, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_505_VERSION_NOT_SUPPORTED, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_400_BAD_REQUEST, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_401_UNAUTHORIZED, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_403_FORBIDDEN, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_404_NOT_FOUND, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_405_METHOD_NOT_ALLOWED, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_408_REQ_TIMEOUT, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    err = httpd_register_err_handler(http_server, HTTPD_411_LENGTH_REQUIRED, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }
    
    err = httpd_register_err_handler(http_server, HTTPD_414_URI_TOO_LONG, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }
    
    err = httpd_register_err_handler(http_server, HTTPD_431_REQ_HDR_FIELDS_TOO_LARGE, http_server_error_handler);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER ERROR HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("WN HTTP SERVER", "Registering URI Handlers");
    #endif


    // Register URI handlers
    err = httpd_register_uri_handler(http_server, &http_server_root_uri);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER URI HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }
    err = httpd_register_uri_handler(http_server, &http_server_form_post_uri);
    if (err != ESP_OK)
    {
        #if (WN_SERVER_LOG_ENABLED)
        ESP_LOGE("WN HTTP SERVER", "HTTP SERVER URI HANDLER REGISTER ERROR: %s", esp_err_to_name(err));
        #endif
    }

    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("WN HTTP SERVER", "Starting Server");
    #endif 
    
    return ESP_OK;
}

esp_err_t wn_server_stop()
{
    // Stop the httpd server
    if (http_server)
    {
        return httpd_stop(http_server);
    }
    else
    {
        return ESP_ERR_INVALID_ARG;
    }
}

//******************** Inıt-Config Functions *****************


//******************** Main Functions ************************



//******************** Get-Set Functions *********************



//******************** Utility Functions **********************

// TODO: Doğrulama işlemi regex ile daha isabetli yapılabilir

static uint32_t wn_search_and_save_nvs(const char* key, const char* value)
{
    uint32_t err_state = 0;
    static bool iot_save_state = false;

    // key aranacak, value üzerinde denetleme yapılacak 
    if (strncmp(key, WN_SERVER_NVS_CHECK_KEY, MAX_NVS_VALUE_LEN) == 0) // güvenlik önlemi olması açısından gizli keyi save state olarak belirledim. 
    {
        iot_save_state = true;
    }
    else if (strncmp(key, WN_SERVER_NVS_AP_NAME_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)

        size_t ap_ssid_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (ap_ssid_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (ap_ssid_len < 2)
        {
            err_state |= NVS_PARAM_SSID_LEN_SHORT;
        }
        else if (ap_ssid_len > 32)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }
        
        for (int i = 0; i < ap_ssid_len; i++)
        {
            if (!isprint((int)value[i]))
            {
                err_state |= NVS_PARAM_NON_PRINTABLE_VALUE;
            }
        }

        #endif

    }
    else if (strncmp(key, WN_SERVER_NVS_AP_PASS_KEY, MAX_NVS_VALUE_LEN) == 0)
    { 
        // Burada accees point eğer checkbox ile açık ağ olarak belirtilmemişse en az 8 karakter şifre olmalıu
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)
        size_t ap_pass_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (ap_pass_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (ap_pass_len < 8)
        {
            err_state |= NVS_PARAM_PASS_LEN_SHORT;
        }
        else if (ap_pass_len > MAX_NVS_VALUE_LEN - 1)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }

        // Şifrenin gözümüzle gördüğümüz bir değer olması lazım!
        for(int i = 0; i < ap_pass_len; i++) //! Türkçe karakterde patlıyor
        {
            if (!isprint((int)value[i]))
            {
                err_state |= NVS_PARAM_NON_PRINTABLE_VALUE;
            }
        }
        #endif 

    }
    else if (strncmp(key, WN_SERVER_NVS_STA_SSID_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)

        size_t sta_ssid_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (sta_ssid_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (sta_ssid_len < 2)
        {
            err_state |= NVS_PARAM_SSID_LEN_SHORT;
        }
        else if (sta_ssid_len > 32)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }

        for (int i = 0; i < sta_ssid_len; i++)
        {
            if(!isprint((int)value[i]))
            {
                err_state |= NVS_PARAM_NON_PRINTABLE_VALUE;
            }
        }

        #endif
    }
    else if (strncmp(key, WN_SERVER_NVS_STA_PASS_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)
        size_t sta_pass_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (sta_pass_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (sta_pass_len < 8)
        {
            err_state |= NVS_PARAM_PASS_LEN_SHORT;
        }
        else if (sta_pass_len > MAX_NVS_VALUE_LEN - 1)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }

        for (int i = 0; i < sta_pass_len; i++)
        {
            if (!isprint((int)value[i]))
            {
                err_state |= NVS_PARAM_NON_PRINTABLE_VALUE;
            }
        }
        #endif
    }

    #if(WN_SERVER_MQTT_CONFIG_ENABLED)

    else if (strncmp(key, WN_SERVER_NVS_MQTT_SERVER_NAME_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)
        size_t mqtt_server_name_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (mqtt_server_name_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (mqtt_server_name_len > MAX_NVS_VALUE_LEN - 1)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }
        #endif 
    }
    else if (strncmp(key, WN_SERVER_NVS_MQTT_SERVER_PORT_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)
        size_t mqtt_server_port_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (mqtt_server_port_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (mqtt_server_port_len > 5)
        {
            err_state |= NVS_PARAM_MQTT_PORT_LEN_LONG;
        }

        for (int i = 0; i < mqtt_server_port_len; i++)
        {
            if(!isdigit((int)value[i]))
            {
                err_state |= NVS_PARAM_NON_NUMERIC_VALUE;
            }
        }
        #endif
    }
    else if (strncmp(key, WN_SERVER_NVS_MQTT_USER_NAME_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)
        size_t mqtt_user_name_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (mqtt_user_name_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (mqtt_user_name_len > MAX_NVS_VALUE_LEN - 1)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }
        #endif
    }
    else if (strncmp(key, WN_SERVER_NVS_MQTT_USER_PASS_KEY, MAX_NVS_VALUE_LEN) == 0)
    {
        #if (WN_SERVER_FORM_PARAM_CHECK_ENABLED)
        size_t mqtt_user_pass_len = strnlen(value, MAX_NVS_VALUE_LEN);
        if (mqtt_user_pass_len == 0)
        {
            err_state |= NVS_PARAM_EMPTY_VALUE;
        }
        else if (mqtt_user_pass_len > MAX_NVS_VALUE_LEN - 1)
        {
            err_state |= NVS_PARAM_SSID_LEN_LONG;
        }
        #endif
    }

    #endif // WN_SERVER_MQTT_CONFIG_ENABLED
    
    else
    {
        ESP_LOGE("Param Check", "Key %s Not Found", key);
        err_state = NVS_PARAM_KEY_ERROR;
    }

    #if (WN_SERVER_LOG_ENABLED)
    ESP_LOGI("NVS", "Key %s Found Value %s", key, value);
    #endif

    if (err_state == 0 && iot_save_state == true)
    {
        wn_nvs_set_str(key, value);
    }

    return err_state;
}



