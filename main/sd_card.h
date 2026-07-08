/*
 * sd_card.h
 * -----------------------------------------------------------------------
 * Opsiyonel SD kart destegi (erisim loglarini dosyaya yazmak icin).
 * CONFIG_APP_ENABLE_SD_CARD kapaliyken bu modulun fonksiyonlari hicbir sey
 * yapmadan ESP_OK/false doner; SD kart olmadan sistem sorunsuz calisir.
 */
#pragma once

#include "esp_err.h"

/* SPI modunda SD karti /sdcard noktasina mount etmeye calisir. Kart yoksa
 * veya CONFIG_APP_ENABLE_SD_CARD kapaliysa basarisiz doner ama uygulamayi
 * durdurmaz (opsiyonel ozellik). */
esp_err_t sd_card_init(void);

bool sd_card_is_mounted(void);
