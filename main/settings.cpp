#include "settings.h"
#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "settings";
static const char *NVS_NAMESPACE = "app_settings";
static const char *KEY_THRESHOLD = "threshold";
static const char *KEY_GRANT_SEC = "grant_sec";

static float s_threshold = SETTINGS_DEFAULT_THRESHOLD;
static uint32_t s_grant_seconds = SETTINGS_DEFAULT_GRANT_SECONDS;

esp_err_t settings_init(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK) {
        uint32_t raw_threshold_milli = 0;
        if (nvs_get_u32(handle, KEY_THRESHOLD, &raw_threshold_milli) == ESP_OK) {
            s_threshold = raw_threshold_milli / 1000.0f;
        }
        nvs_get_u32(handle, KEY_GRANT_SEC, &s_grant_seconds);
        nvs_close(handle);
    } else {
        ESP_LOGI(TAG, "Kayitli ayar bulunamadi, varsayilanlar kullaniliyor.");
    }

    ESP_LOGI(TAG, "Esik=%.2f, izin suresi=%u sn", s_threshold, (unsigned)s_grant_seconds);
    return ESP_OK;
}

float settings_get_threshold(void)
{
    return s_threshold;
}

esp_err_t settings_set_threshold(float threshold)
{
    if (threshold < 0.0f || threshold > 1.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    s_threshold = threshold;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }
    /* Instead of storing a float in NVS we use a milli-unit (thousandths)
     * integer; that works reliably across all ESP-IDF versions. */
    err = nvs_set_u32(handle, KEY_THRESHOLD, (uint32_t)(threshold * 1000.0f));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

uint32_t settings_get_grant_seconds(void)
{
    return s_grant_seconds;
}

esp_err_t settings_set_grant_seconds(uint32_t seconds)
{
    if (seconds == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    s_grant_seconds = seconds;

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_u32(handle, KEY_GRANT_SEC, seconds);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
