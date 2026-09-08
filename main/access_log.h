/*
 * access_log.h
 * -----------------------------------------------------------------------
 * Prints every recognition attempt to the serial port; when an SD card is
 * present (sd_card.h) it appends the same line to /sdcard/access_log.csv.
 */
#pragma once

#include "esp_err.h"

esp_err_t access_log_init(void);

/* operator_id: pass -1 when there is no match. operator_name: "unknown" when
 * there is no match. */
void access_log_record(bool granted, int operator_id, const char *operator_name, float similarity);
