/*
 * sd_card.h
 * -----------------------------------------------------------------------
 * Optional SD card support (for writing access logs to a file).
 * When CONFIG_APP_ENABLE_SD_CARD is disabled, this module's functions return
 * ESP_OK/false without doing anything; the system works fine without an SD
 * card.
 */
#pragma once

#include "esp_err.h"

/* Tries to mount the SD card at /sdcard in SPI mode. Fails if there is no
 * card or CONFIG_APP_ENABLE_SD_CARD is disabled, but does not halt the
 * application (it is an optional feature). */
esp_err_t sd_card_init(void);

bool sd_card_is_mounted(void);
