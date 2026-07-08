/*
 * operator_store.h
 * -----------------------------------------------------------------------
 * Kayitli operatorleri (id, isim, yuz oznitelik vektoru, kayit zamani)
 * dahili flash uzerindeki ayri bir FATFS partition'inda ("facedb") sabit
 * boyutlu kayitlar halinde saklar.
 *
 * NVS yerine ayri bir FATFS partition secilmesinin nedeni: her oznitelik
 * vektoru ~2KB (512 float) tutar; NVS blob'lari bu boyutta tekrarlanan
 * yazimda pratik degildir. Sabit kayitli basit bir dosya, ogrencilere
 * "veritabani" mantigini da somut olarak gosterir.
 */
#pragma once

#include <cstdint>
#include "esp_err.h"

#define OPERATOR_MAX_COUNT   10
#define OPERATOR_NAME_LEN     32
/* HumanFaceFeat (MFN_S8_V1) modelinin oznitelik vektoru uzunlugu.
 * face_engine_init() bunu gercek modelden okuyup dogrular; uyusmazsa
 * hata loglar (bkz. face_engine.cpp). */
#define OPERATOR_FEAT_LEN    512

struct operator_record_t {
    uint8_t id;
    char name[OPERATOR_NAME_LEN];
    float feat[OPERATOR_FEAT_LEN];
    uint32_t enrolled_at_s; /* esp_timer_get_time() tabanli, saniye */
};

/* "facedb" partition'ini FATFS olarak mount eder (ilk calistirmada
 * formatlar) ve varsa kayitli operatorleri RAM onbellegine yukler. */
esp_err_t operator_store_init(void);

int operator_store_count(void);

/* index: 0..operator_store_count()-1. Sinir disi ise nullptr doner. */
const operator_record_t *operator_store_get_by_index(int index);

const operator_record_t *operator_store_find(uint8_t id);

/* Yeni operator ekler (depo doluysa ESP_ERR_NO_MEM doner). Basariliysa
 * out_id'ye atanan id yazilir. */
esp_err_t operator_store_add(const char *name, const float *feat, int feat_len, uint8_t *out_id);

esp_err_t operator_store_delete(uint8_t id);
