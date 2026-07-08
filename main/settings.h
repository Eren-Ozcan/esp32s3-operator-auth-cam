/*
 * settings.h
 * -----------------------------------------------------------------------
 * Kalici (NVS destekli) yapilandirma: benzerlik esigi ve izin suresi.
 * serial_cli'daki set-threshold / set-duration komutlariyla degistirilir
 * ve reset sonrasi da korunur.
 */
#pragma once

#include "esp_err.h"

#define SETTINGS_DEFAULT_THRESHOLD    0.55f  /* Cosine similarity esigi; sinifta test edip ayarlayin. */
#define SETTINGS_DEFAULT_GRANT_SECONDS 5      /* Izin GPIO'sunun acik kalacagi sure (sn). */

/* NVS'yi baslatir (esp_nvs flash init'in kendisi app_main'de yapilir, bu
 * fonksiyon sadece ilgili namespace'i acar) ve kayitli degerleri okur;
 * yoksa varsayilanlari kullanir. */
esp_err_t settings_init(void);

float settings_get_threshold(void);
esp_err_t settings_set_threshold(float threshold); /* 0.0-1.0 araligi disinda ESP_ERR_INVALID_ARG doner */

uint32_t settings_get_grant_seconds(void);
esp_err_t settings_set_grant_seconds(uint32_t seconds); /* 0 gecersiz */
