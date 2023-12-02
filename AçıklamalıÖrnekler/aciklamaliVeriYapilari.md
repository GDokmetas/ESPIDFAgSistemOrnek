# Açıklamalı Veri Yapıları

Bu belgede geliştirmede işimize yarayacak ve .h dosyalarında mevcut olan veri yapıları ve açıklamaları yer almaktadır.

## Dosya "esp_http_client.h"



**HTTP Events**


```c
typedef enum {
    HTTP_EVENT_ERROR = 0,       // Veri alışverişinde herhangi bir sorun yaşandı
    HTTP_EVENT_ON_CONNECTED,    //*Sunucuya bağlanıldı, veri alışverişi yaşanmadı. 
    HTTP_EVENT_HEADERS_SENT,     // Sunucuya bütün headerlar gönderildi
    HTTP_EVENT_ON_HEADER,       // Sunucudan alınan her header için tetiklenir
    HTTP_EVENT_ON_DATA,         // Sunucudan alınan her veri için tetiklenir
    HTTP_EVENT_ON_FINISH,       // HTTP sezonu bitirildiğinde tetiklenir
    HTTP_EVENT_DISCONNECTED,    // Bağlantıdan kopulduğunda tetiklenir
    HTTP_EVENT_REDIRECT,        // HTTP yeniden yönlendirmeleri manuel olarak ele almak için kullanılır.
} esp_http_client_event_id_t;
```

Olaylar gerçekleştiği zaman gerçekleşen olaylara göre data içerisindeki veri tipi değişmektedir. Eventları ele alırken bu verileri kullanmak gereklidir.

HTTP_EVENT_ON_DATA 'da alınan veri,

```c
typedef struct esp_http_client_on_data {
    esp_http_client_handle_t client;    /*!< Client handle */
    int64_t data_process;               /*!< Total data processed */
} esp_http_client_on_data_t;

```

HTTP_EVENT_REDIRECT'de alınan veri,

```c
typedef struct esp_http_client_redirect_event_data {
    esp_http_client_handle_t client;    /*!< Client handle */
    int status_code;                    /*!< Status Code */
} esp_http_client_redirect_event_data_t;
```

**HTTP Client Hata Kodları ve Açıklamaları**

```c
#define ESP_ERR_HTTP_MAX_REDIRECT       (ESP_ERR_HTTP_BASE + 1)     // Azami yeniden yönlendirme aşıldı
#define ESP_ERR_HTTP_CONNECT            (ESP_ERR_HTTP_BASE + 2)     // HTTP bağlantısı açılamadı
#define ESP_ERR_HTTP_WRITE_DATA         (ESP_ERR_HTTP_BASE + 3)     // HTTP verisi yazılamadı
#define ESP_ERR_HTTP_FETCH_HEADER       (ESP_ERR_HTTP_BASE + 4)     // Sunucudan header alınamadı
#define ESP_ERR_HTTP_INVALID_TRANSPORT  (ESP_ERR_HTTP_BASE + 5)     // Giriş şemasına göre geçersiz HTTP taşıyıcısı
#define ESP_ERR_HTTP_CONNECTING         (ESP_ERR_HTTP_BASE + 6)     // HTTP bağlantısı kuruluyor
#define ESP_ERR_HTTP_EAGAIN             (ESP_ERR_HTTP_BASE + 7)     /*!< Mapping of errno EAGAIN to esp_err_t */
#define ESP_ERR_HTTP_CONNECTION_CLOSED  (ESP_ERR_HTTP_BASE + 8)     /*!< Read FIN from peer and the connection closed */
```

