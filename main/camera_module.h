/*
 * camera_module.h
 * -----------------------------------------------------------------------
 * OV3660 kamerayi (esp32-camera surucusu uzerinden) baslatir ve yakalanan
 * kareyi ESP-DL'in bekledigi dl::image::img_t formatina cevirir.
 *
 * NOT: Proje tamamen C++ olarak derlendigi icin (app_main dahil) burada
 * extern "C" sarmalamaya gerek yoktur; sadece app_main() giris noktasi
 * ESP-IDF tarafindan C linkage ile aranir (bkz. app_main.cpp).
 */
#pragma once

#include "esp_err.h"
#include "esp_camera.h"
#include "dl_image_define.hpp"

/* Kamerayi pin_config.h'deki pinlerle baslatir. PIXFORMAT_RGB565 kullanilir
 * (JPEG degil) cunku ESP-DL modelleri dogrudan piksel verisi bekler; her
 * karede JPEG cozme adimindan kacinilarak gecikme dusurulur. */
esp_err_t camera_module_init(void);

/* Bir kare yakalar. Donen isaretci camera_module_release() ile geri
 * verilmelidir. Basarisizlikta nullptr doner. */
camera_fb_t *camera_module_capture(void);

/* camera_module_capture() ile alinan tamponu surucuye iade eder. */
void camera_module_release(camera_fb_t *fb);

/* Yakalanan RGB565 kareyi, veriyi KOPYALAMADAN (ayni bellegi isaret ederek)
 * ESP-DL'in img_t yapisina sarmalar. Donen img_t, fb gecerli oldugu surece
 * kullanilabilir; fb serbest birakilmadan once img_t kullanimi bitirilmelidir. */
dl::image::img_t camera_fb_to_img(const camera_fb_t *fb);
