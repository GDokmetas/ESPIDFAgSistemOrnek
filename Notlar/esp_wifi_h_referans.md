# Wifi Referans

Bu belgede wifi fonksiyonlarının kısa açıklamaları yer almaktadır.

```c
    esp_err_t esp_wifi_init(const wifi_init_config_t *config);
```

Wifi init için kullanılmaktadır. Bütün fonksiyonlardan önce bu fonksiyon çağırılmalıdır. WIFI_INIT_CONFIG_DEFAULT() makrosu ile ön tanımlı parametreler aktarılıp sonrasında argümanlar tek tek değiştirilebilir. 

wifi_init_config_t veri yapısında Wifi ile ilgili ayarlar bulunmaktadır. Default olanı aşağıdaki gibidir. Eğer istenirse bu parametreler değiştirilerek sdkconfig'de yer alan ayarlar geçersiz kılınabilir. 

```c
#define WIFI_INIT_CONFIG_DEFAULT() { \
    .osi_funcs = &g_wifi_osi_funcs, \
    .wpa_crypto_funcs = g_wifi_default_wpa_crypto_funcs, \
    .static_rx_buf_num = CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM,\
    .dynamic_rx_buf_num = CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM,\
    .tx_buf_type = CONFIG_ESP_WIFI_TX_BUFFER_TYPE,\
    .static_tx_buf_num = WIFI_STATIC_TX_BUFFER_NUM,\
    .dynamic_tx_buf_num = WIFI_DYNAMIC_TX_BUFFER_NUM,\
    .cache_tx_buf_num = WIFI_CACHE_TX_BUFFER_NUM,\
    .csi_enable = WIFI_CSI_ENABLED,\
    .ampdu_rx_enable = WIFI_AMPDU_RX_ENABLED,\
    .ampdu_tx_enable = WIFI_AMPDU_TX_ENABLED,\
    .amsdu_tx_enable = WIFI_AMSDU_TX_ENABLED,\
    .nvs_enable = WIFI_NVS_ENABLED,\
    .nano_enable = WIFI_NANO_FORMAT_ENABLED,\
    .rx_ba_win = WIFI_DEFAULT_RX_BA_WIN,\
    .wifi_task_core_id = WIFI_TASK_CORE_ID,\
    .beacon_max_len = WIFI_SOFTAP_BEACON_MAX_LEN, \
    .mgmt_sbuf_num = WIFI_MGMT_SBUF_NUM, \
    .feature_caps = g_wifi_feature_caps, \
    .sta_disconnected_pm = WIFI_STA_DISCONNECTED_PM_ENABLED,  \
    .espnow_max_encrypt_num = CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM, \
    .magic = WIFI_INIT_CONFIG_MAGIC\
}

```




```c
    esp_err_t esp_wifi_deinit(void);
```

Wifi kullanılmayacağı zaman deinit yapmaya yaramaktadır. (Hafızada alan açmakta!)

```c
    esp_err_t esp_wifi_set_mode(wifi_mode_t mode);
```

Wifi modunu ayarlamak için kullanılmakta. STA, AP, STA+AP ya da NAN modlarından biri seçilebilir. (Neighborhood Aware Networking)


```c
    esp_err_t esp_wifi_get_mode(wifi_mode_t *mode);
```

Mevcut Wifi modunu almak için kullanılır.

```c
    esp_err_t esp_wifi_start(void);
```

Wifi'ı başlatmak için kullanılır. (Mevcut konfigürasyona göre) ! Buralarda error check yapılmalı.

```c
    esp_err_t esp_wifi_stop(void);
```

Wifi'ı durdurmak için kullanılır.

```c
    esp_err_t esp_wifi_restore(void);
```

Wifi'ın durumlarını sıfırlamak için kullanılır (protocol, bandwidth, mode, config)

```c
    esp_err_t esp_wifi_connect(void);
```

SSID ve Şifresi belirlenmmiş AP'ye bağlanmak için kullanılır. Sadece STA modda geçerli olmaktadır.

```c
    esp_err_t esp_wifi_disconnect(void);
```

Bağlı AP'den kopmak için kullanılmaktadır.

```c
    esp_err_t esp_wifi_clear_fast_connect(void);
```

Boş fonksiyon.

```c
    esp_err_t esp_wifi_deauth_sta(uint16_t aid);
```

Bağlı istemciyi koparmak için kullanılır. aid 0 ise tüm istemcileri koparır.

```c
    esp_err_t esp_wifi_scan_start(const wifi_scan_config_t *config, bool block);
```

Mevcut tüm AP'leri taramayı başlatmak için kullanılır.

```c
    esp_err_t esp_wifi_scan_stop(void);
```

İşlemdeki taramayı durdurmak için kullanılır.

```c
    esp_err_t esp_wifi_scan_get_ap_num(uint16_t *number);
```

Son taramada kaç AP bulunduğunu gösterir.

```c
    esp_err_t esp_wifi_scan_get_ap_records(uint16_t *number, wifi_ap_record_t *ap_records);
```

Son taramadaki AP listesini verir.

```c
    esp_err_t esp_wifi_clear_ap_list(void);
```

Son taramadaki AP listesini silmek için kullanılır

```c
    esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *ap_info);
```

Bağlı bulunan AP'nin bilgilerini almak için kullanılır. 

```c
    esp_err_t esp_wifi_set_ps(wifi_ps_type_t type);
```

Wifi güç tasarrufu tipini belirlemek için kullanılır.

```c
    esp_err_t esp_wifi_get_ps(wifi_ps_type_t *type);
```

Mevcut güç tasarrufu tipini verir.

```c
    esp_err_t esp_wifi_set_protocol(wifi_interface_t ifx, uint8_t protocol_bitmap);
```

Protokol bitmap formatını belirlemek için kullanılır (https://en.wikipedia.org/wiki/Wireless_Application_Protocol_Bitmap_Format)

```c
    esp_err_t esp_wifi_get_protocol(wifi_interface_t ifx, uint8_t *protocol_bitmap);
```

Protokol bitmap formatını belirlemek için kullanılır (https://en.wikipedia.org/wiki/Wireless_Application_Protocol_Bitmap_Format)


```c
    esp_err_t esp_wifi_set_bandwidth(wifi_interface_t ifx, wifi_bandwidth_t bw);
```

Bant genişliğini belirlemek için kullanılır (20Mhz vs 40MHz)

```c
    esp_err_t esp_wifi_get_bandwidth(wifi_interface_t ifx, wifi_bandwidth_t *bw);
```

Bant genişliğini belirlemek için kullanılır (20Mhz vs 40MHz)

```c
    esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
```

Birincil ve ikincil kanalı belirlemek için kullanılır. 

```c
    esp_err_t esp_wifi_get_channel(uint8_t *primary, wifi_second_chan_t *second);
```

Seçili kanalı verir. 

```c
    esp_err_t esp_wifi_set_country(const wifi_country_t *country);
```

Ülke bazlı kısıtların bulunduğu parametreleri ayarlamak için kullanılır.

```c
    esp_err_t esp_wifi_get_country(wifi_country_t *country);
```

```c
    esp_err_t esp_wifi_set_mac(wifi_interface_t ifx, const uint8_t mac[6]);
```

Arayüzlerin mac adresini belirlemek için kullanılır

```c
    esp_err_t esp_wifi_get_mac(wifi_interface_t ifx, uint8_t mac[6]);
```

Arayüzlerin mac adresini almak için kullanılır.

```c
    typedef void (* wifi_promiscuous_cb_t)(void *buf, wifi_promiscuous_pkt_type_t type);
```

Promiscuous mod ile ilgili ayarlar----------------


```c
    esp_err_t esp_wifi_set_promiscuous_rx_cb(wifi_promiscuous_cb_t cb);
```

```c
    esp_err_t esp_wifi_set_promiscuous(bool en);
```

```c
    esp_err_t esp_wifi_get_promiscuous(bool *en);
```

```c
    esp_err_t esp_wifi_set_promiscuous_filter(const wifi_promiscuous_filter_t *filter);
```

```c
    esp_err_t esp_wifi_get_promiscuous_filter(wifi_promiscuous_filter_t *filter);
```

```c
    esp_err_t esp_wifi_set_promiscuous_ctrl_filter(const wifi_promiscuous_filter_t *filter);
```

```c
    esp_err_t esp_wifi_get_promiscuous_ctrl_filter(wifi_promiscuous_filter_t *filter);
```
-------------------------


```c
    esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf);
```

```c
    esp_err_t esp_wifi_get_config(wifi_interface_t interface, wifi_config_t *conf);
```

```c
    esp_err_t esp_wifi_ap_get_sta_list(wifi_sta_list_t *sta);
```

```c
    esp_err_t esp_wifi_ap_get_sta_aid(const uint8_t mac[6], uint16_t *aid);
```

```c
    esp_err_t esp_wifi_set_storage(wifi_storage_t storage);
```

```c
    typedef void (*esp_vendor_ie_cb_t) (void *ctx, wifi_vendor_ie_type_t type, const uint8_t sa[6], const vendor_ie_data_t *vnd_ie, int rssi);
```

```c
    esp_err_t esp_wifi_set_vendor_ie(bool enable, wifi_vendor_ie_type_t type, wifi_vendor_ie_id_t idx, const void *vnd_ie);
```

```c
    esp_err_t esp_wifi_set_vendor_ie_cb(esp_vendor_ie_cb_t cb, void *ctx);
```

```c
    esp_err_t esp_wifi_set_max_tx_power(int8_t power);
```

```c
    esp_err_t esp_wifi_get_max_tx_power(int8_t *power);
```

```c
    esp_err_t esp_wifi_set_event_mask(uint32_t mask);
```

```c
    esp_err_t esp_wifi_get_event_mask(uint32_t *mask);
```

```c
    esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
```

```c
    typedef void (* wifi_csi_cb_t)(void *ctx, wifi_csi_info_t *data);
```

```c
    esp_err_t esp_wifi_set_csi_rx_cb(wifi_csi_cb_t cb, void *ctx);
```

```c
    esp_err_t esp_wifi_set_csi_config(const wifi_csi_config_t *config);
```

```c
    esp_err_t esp_wifi_set_csi(bool en);
```

```c
    esp_err_t esp_wifi_set_ant_gpio(const wifi_ant_gpio_config_t *config);
```

```c
    esp_err_t esp_wifi_get_ant_gpio(wifi_ant_gpio_config_t *config);
```

```c
    esp_err_t esp_wifi_set_ant(const wifi_ant_config_t *config);
```

```c
    esp_err_t esp_wifi_get_ant(wifi_ant_config_t *config);
```

```c
    int64_t esp_wifi_get_tsf_time(wifi_interface_t interface);
```

```c
    esp_err_t esp_wifi_set_inactive_time(wifi_interface_t ifx, uint16_t sec);
```

```c
    esp_err_t esp_wifi_get_inactive_time(wifi_interface_t ifx, uint16_t *sec);
```

```c
    esp_err_t esp_wifi_statis_dump(uint32_t modules);
```

```c
    esp_err_t esp_wifi_set_rssi_threshold(int32_t rssi);
```

```c
    esp_err_t esp_wifi_ftm_initiate_session(wifi_ftm_initiator_cfg_t *cfg);
```

```c
    esp_err_t esp_wifi_ftm_end_session(void);
```

```c
    esp_err_t esp_wifi_ftm_resp_set_offset(int16_t offset_cm);
```

```c
    esp_err_t esp_wifi_config_11b_rate(wifi_interface_t ifx, bool disable);
```

```c
    esp_err_t esp_wifi_connectionless_module_set_wake_interval(uint16_t wake_interval);
```

```c
    esp_err_t esp_wifi_set_country_code(const char *country, bool ieee80211d_enabled);
```

```c
    esp_err_t esp_wifi_get_country_code(char *country);
```

```c
    esp_err_t esp_wifi_config_80211_tx_rate(wifi_interface_t ifx, wifi_phy_rate_t rate);
```

```c
    esp_err_t esp_wifi_disable_pmf_config(wifi_interface_t ifx);
```

```c
    esp_err_t esp_wifi_sta_get_aid(uint16_t *aid);
```

```c
    esp_err_t esp_wifi_sta_get_negotiated_phymode(wifi_phy_mode_t *phymode);
```

```c
    esp_err_t esp_wifi_set_dynamic_cs(bool enabled);
```

```c
    esp_err_t esp_wifi_sta_get_rssi(int *rssi);
```