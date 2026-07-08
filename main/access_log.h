/*
 * access_log.h
 * -----------------------------------------------------------------------
 * Her tanima denemesini seri porta basar; SD kart takiliysa (sd_card.h)
 * ayni satiri /sdcard/access_log.csv dosyasina da ekler.
 */
#pragma once

#include "esp_err.h"

esp_err_t access_log_init(void);

/* operator_id: eslesme yoksa -1 gecin. operator_name: eslesme yoksa "bilinmiyor". */
void access_log_record(bool granted, int operator_id, const char *operator_name, float similarity);
