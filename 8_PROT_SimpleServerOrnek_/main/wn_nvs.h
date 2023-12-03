#ifndef WIFI_NETWORK_NVS_H
#define WIFI_NETWORK_NVS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define NVS_NAMESPACE "data"
#include "esp_err.h"
// Veri keyleri app tarafında olacak



esp_err_t wn_nvs_get_int8(const char* key, int8_t* value);
esp_err_t wn_nvs_get_int16(const char* key, int16_t* value);
esp_err_t wn_nvs_get_int32(const char* key, int32_t* value);
esp_err_t wn_nvs_get_int64(const char* key, int64_t* value);
esp_err_t wn_nvs_get_uint8(const char* key, uint8_t* value);
esp_err_t wn_nvs_get_uint16(const char* key, uint16_t* value);
esp_err_t wn_nvs_get_uint32(const char* key, uint32_t* value);
esp_err_t wn_nvs_get_uint64(const char* key, uint64_t* value);
esp_err_t wn_nvs_get_str(const char* key, char* value, size_t length);

esp_err_t wn_nvs_set_int8(const char* key, int8_t value);
esp_err_t wn_nvs_set_int16(const char* key, int16_t value);
esp_err_t wn_nvs_set_int32(const char* key, int32_t value);
esp_err_t wn_nvs_set_int64(const char* key, int64_t value);
esp_err_t wn_nvs_set_uint8(const char* key, uint8_t value);
esp_err_t wn_nvs_set_uint16(const char* key, uint16_t value);
esp_err_t wn_nvs_set_uint32(const char* key, uint32_t value);
esp_err_t wn_nvs_set_uint64(const char* key, uint64_t value);
esp_err_t wn_nvs_set_str(const char* key, const char* value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // WIFI_NETWORK_NVS_H