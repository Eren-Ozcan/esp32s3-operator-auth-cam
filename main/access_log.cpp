#include "access_log.h"
#include "sd_card.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <cstdio>

static const char *TAG = "access_log";
static const char *LOG_FILE = "/sdcard/access_log.csv";

esp_err_t access_log_init(void)
{
    if (sd_card_is_mounted()) {
        /* Create the file with a header row when it does not exist. */
        FILE *f = fopen(LOG_FILE, "r");
        if (f == NULL) {
            f = fopen(LOG_FILE, "w");
            if (f != NULL) {
                fprintf(f, "zaman_s,sonuc,operator_id,operator_isim,benzerlik\n");
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
    return ESP_OK;
}

void access_log_record(bool granted, int operator_id, const char *operator_name, float similarity)
{
    uint32_t now_s = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    const char *result_str = granted ? "IZIN_VERILDI" : "REDDEDILDI";

    ESP_LOGI(TAG, "[%u] %s id=%d isim=%s benzerlik=%.2f", (unsigned)now_s, result_str, operator_id, operator_name,
             similarity);

    if (sd_card_is_mounted()) {
        FILE *f = fopen(LOG_FILE, "a");
        if (f != NULL) {
            fprintf(f, "%u,%s,%d,%s,%.2f\n", (unsigned)now_s, result_str, operator_id, operator_name, similarity);
            fclose(f);
        } else {
            ESP_LOGW(TAG, "Log dosyasi acilamadi, sadece seri porta yazildi.");
        }
    }
}
