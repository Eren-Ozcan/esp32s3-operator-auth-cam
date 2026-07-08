#include "operator_store.h"
#include <cstdio>
#include <cstring>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "esp_timer.h"

static const char *TAG = "operator_store";
static const char *MOUNT_POINT = "/facedb";
static const char *DB_FILE = "/facedb/operators.bin";

static operator_record_t s_records[OPERATOR_MAX_COUNT];
static int s_count = 0;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

static void save_to_file(void)
{
    FILE *f = fopen(DB_FILE, "wb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Kayit dosyasi yazilamadi: %s", DB_FILE);
        return;
    }
    fwrite(&s_count, sizeof(s_count), 1, f);
    if (s_count > 0) {
        fwrite(s_records, sizeof(operator_record_t), s_count, f);
    }
    fclose(f);
}

static void load_from_file(void)
{
    FILE *f = fopen(DB_FILE, "rb");
    if (f == NULL) {
        ESP_LOGI(TAG, "Kayitli operator dosyasi yok, bos baslaniyor.");
        s_count = 0;
        return;
    }
    int count = 0;
    if (fread(&count, sizeof(count), 1, f) != 1 || count < 0 || count > OPERATOR_MAX_COUNT) {
        ESP_LOGW(TAG, "Kayit dosyasi bozuk gorunuyor, bos baslaniyor.");
        s_count = 0;
        fclose(f);
        return;
    }
    size_t read_n = fread(s_records, sizeof(operator_record_t), count, f);
    fclose(f);
    s_count = (int)read_n;
    ESP_LOGI(TAG, "%d operator yuklendi.", s_count);
}

esp_err_t operator_store_init(void)
{
    esp_vfs_fat_mount_config_t mount_config = {};
    mount_config.max_files = 2;
    mount_config.format_if_mount_failed = true;
    mount_config.allocation_unit_size = 4096;

    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_POINT, "facedb", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "facedb partition mount edilemedi: 0x%x", err);
        return err;
    }

    load_from_file();
    return ESP_OK;
}

int operator_store_count(void)
{
    return s_count;
}

const operator_record_t *operator_store_get_by_index(int index)
{
    if (index < 0 || index >= s_count) {
        return NULL;
    }
    return &s_records[index];
}

const operator_record_t *operator_store_find(uint8_t id)
{
    for (int i = 0; i < s_count; i++) {
        if (s_records[i].id == id) {
            return &s_records[i];
        }
    }
    return NULL;
}

static uint8_t next_free_id(void)
{
    for (uint8_t candidate = 0; candidate < OPERATOR_MAX_COUNT; candidate++) {
        bool used = false;
        for (int i = 0; i < s_count; i++) {
            if (s_records[i].id == candidate) {
                used = true;
                break;
            }
        }
        if (!used) {
            return candidate;
        }
    }
    return OPERATOR_MAX_COUNT; /* dolu isareti */
}

esp_err_t operator_store_add(const char *name, const float *feat, int feat_len, uint8_t *out_id)
{
    if (s_count >= OPERATOR_MAX_COUNT) {
        ESP_LOGW(TAG, "Maksimum operator sayisina (%d) ulasildi.", OPERATOR_MAX_COUNT);
        return ESP_ERR_NO_MEM;
    }
    if (feat_len != OPERATOR_FEAT_LEN) {
        ESP_LOGE(TAG, "Beklenmeyen oznitelik vektoru uzunlugu: %d (beklenen %d)", feat_len, OPERATOR_FEAT_LEN);
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t id = next_free_id();
    if (id >= OPERATOR_MAX_COUNT) {
        return ESP_ERR_NO_MEM;
    }

    operator_record_t *rec = &s_records[s_count];
    rec->id = id;
    strncpy(rec->name, name, OPERATOR_NAME_LEN - 1);
    rec->name[OPERATOR_NAME_LEN - 1] = '\0';
    memcpy(rec->feat, feat, sizeof(float) * OPERATOR_FEAT_LEN);
    rec->enrolled_at_s = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    s_count++;

    save_to_file();

    if (out_id != NULL) {
        *out_id = id;
    }
    ESP_LOGI(TAG, "Operator kaydedildi: id=%d isim=%s", id, rec->name);
    return ESP_OK;
}

esp_err_t operator_store_delete(uint8_t id)
{
    for (int i = 0; i < s_count; i++) {
        if (s_records[i].id == id) {
            /* Silinen kaydin yerini son kayitla doldur (sirali olmasi
             * onemli degil, id her zaman kayit icinde saklaniyor). */
            s_records[i] = s_records[s_count - 1];
            s_count--;
            save_to_file();
            ESP_LOGI(TAG, "Operator silindi: id=%d", id);
            return ESP_OK;
        }
    }
    ESP_LOGW(TAG, "Silinecek operator bulunamadi: id=%d", id);
    return ESP_ERR_NOT_FOUND;
}
