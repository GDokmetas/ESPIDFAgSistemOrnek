#include "wn_nvs.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_log.h"
nvs_handle_t wn_nvs_handle;

static esp_err_t wn_nvs_init(void)
{
    esp_err_t err = ESP_OK;
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGE("NVS", "NVS flash init failed! Erasing flash...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        ESP_ERROR_CHECK(err);
    }

    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS flash init failed!");
    }
    return err;
}

static esp_err_t wn_nvs_open(void)
{
    esp_err_t err = ESP_OK;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &wn_nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS open failed!");
    }
    return err;
}

static esp_err_t wn_nvs_commit(void)
{
    esp_err_t err = ESP_OK;
    err = nvs_commit(wn_nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS commit failed!");
    }
    return err;
}

static esp_err_t wn_nvs_close(void)
{
    esp_err_t err = ESP_OK;
    nvs_close(wn_nvs_handle);
    return err;
}

static void check_read_error(esp_err_t err)
{
    switch(err)
    {
        case ESP_FAIL:
            ESP_LOGE("NVS", "Generic ESP_FAIL (Prob. Corrupted NVS Partition)");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE("NVS", "Requested key not found");
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGE("NVS", "NVS handle is invalid");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGE("NVS", "Requested key is invalid");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGE("NVS", "Requested key length is invalid");
            break;
        default:
            ESP_LOGE("NVS", "Unknown error");
            break;
    }
}
// NVS Get Fonksiyonları

esp_err_t wn_nvs_get_int8(const char* key, int8_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_i8(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_uint8(const char* key, uint8_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_u8(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_int16(const char* key, int16_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_i16(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int16 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_uint16(const char* key, uint16_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_u16(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_int32(const char* key, int32_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_i32(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_uint32(const char* key, uint32_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_u32(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_int64(const char* key, int64_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_i64(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_uint64(const char* key, uint64_t* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_u64(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_get_str(const char* key, char* value, size_t len)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_str(wn_nvs_handle, key, value, &len);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS get int8 failed!");
        check_read_error(err);
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();
    return err;
}

// NVS Set fonksiyonları

esp_err_t wn_nvs_set_int8(const char* key, int8_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_i8(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set int8 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();

    return err;
}

esp_err_t wn_nvs_set_uint8(const char* key, uint8_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_u8(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set uint8 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();

    return err;
}

esp_err_t wn_nvs_set_int16(const char* key, int16_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_i16(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set int16 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();

    return err;
}


esp_err_t wn_nvs_set_uint16(const char* key, uint16_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_u16(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set uint16 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();

    return err;
}

esp_err_t wn_nvs_set_int32(const char* key, int32_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_i32(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set int32 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();

    return err;
}

esp_err_t wn_nvs_set_uint32(const char* key, uint32_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_u32(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set uint32 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }
    wn_nvs_close();

    return err;
}

esp_err_t wn_nvs_set_int64(const char* key, int64_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err; 
    }

    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_i64(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set int64 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }

    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_set_uint64(const char* key, uint64_t value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }
    
    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_u64(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set uint64 failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }

    wn_nvs_close();
    return err;
}

esp_err_t wn_nvs_set_str(const char* key, const char* value)
{
    esp_err_t err = ESP_OK;
    
    err = wn_nvs_init();
    if (err != ESP_OK)
    {
        return err;
    }
    
    err = wn_nvs_open();
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_str(wn_nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "NVS set str failed!");
        return err;
    }

    err = wn_nvs_commit();
    if (err != ESP_OK)
    {
        return err;
    }

    wn_nvs_close();
    return err;
}
