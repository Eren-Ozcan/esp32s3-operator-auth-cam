/*
 * settings.h
 * -----------------------------------------------------------------------
 * Persistent (NVS-backed) configuration: similarity threshold and permission
 * duration. Changed with the set-threshold / set-duration commands in
 * serial_cli, and preserved across resets.
 */
#pragma once

#include "esp_err.h"

#define SETTINGS_DEFAULT_THRESHOLD    0.55f  /* Cosine similarity threshold; test in class and tune. */
#define SETTINGS_DEFAULT_GRANT_SECONDS 5      /* How long the permission GPIO stays on (s). */

/* Initializes NVS (the esp_nvs flash init itself happens in app_main; this
 * function only opens the relevant namespace) and reads the stored values,
 * falling back to the defaults when there are none. */
esp_err_t settings_init(void);

float settings_get_threshold(void);
esp_err_t settings_set_threshold(float threshold); /* Returns ESP_ERR_INVALID_ARG outside the 0.0-1.0 range */

uint32_t settings_get_grant_seconds(void);
esp_err_t settings_set_grant_seconds(uint32_t seconds); /* 0 is invalid */
