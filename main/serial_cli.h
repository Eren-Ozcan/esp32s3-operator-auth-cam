/*
 * serial_cli.h
 * -----------------------------------------------------------------------
 * esp_console tabanli komut satiri: enroll <isim>, delete <id>, list,
 * set-threshold <deger>, set-duration <saniye>, status.
 *
 * Kamera ve yuz tanima motoru tek bir ana dongu gorevinde (app_main.cpp)
 * calistigi icin "enroll" komutu kayit islemini KENDISI YAPMAZ; sadece
 * bir istek birakir. Ana dongu, bir sonraki karede bu istegi gorup
 * kaydi gerceklestirir (kamera erisiminin tek bir yerden yapilmasini
 * saglar, yaris durumlarini onler).
 */
#pragma once

#include "esp_err.h"
#include <cstddef>

esp_err_t serial_cli_init(void);

/* Konsol komutundan veya buton kesme isleyicisinden cagrilir. Zaten
 * bekleyen bir istek varsa yenisi yoksayilir (log basilir). */
void serial_cli_request_enroll(const char *name);

/* Ana dongu tarafindan her iterasyonda cagrilir. Bekleyen istek varsa
 * name_out'a kopyalar, true doner ve istegi tuketir. */
bool serial_cli_take_enroll_request(char *name_out, size_t max_len);
