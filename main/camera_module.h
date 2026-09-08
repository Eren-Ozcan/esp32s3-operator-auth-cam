/*
 * camera_module.h
 * -----------------------------------------------------------------------
 * Initializes the OV3660 camera (through the esp32-camera driver) and
 * converts a captured frame into the dl::image::img_t format ESP-DL expects.
 *
 * NOTE: the whole project is compiled as C++ (including app_main), so no
 * extern "C" wrapping is needed here; only the app_main() entry point is
 * looked up with C linkage by ESP-IDF (see app_main.cpp).
 */
#pragma once

#include "esp_err.h"
#include "esp_camera.h"
#include "dl_image_define.hpp"

/* Initializes the camera with the pins from pin_config.h. PIXFORMAT_RGB565 is
 * used (not JPEG) because ESP-DL models expect raw pixel data; skipping a JPEG
 * decode step on every frame lowers latency. */
esp_err_t camera_module_init(void);

/* Captures one frame. The returned pointer must be given back with
 * camera_module_release(). Returns nullptr on failure. */
camera_fb_t *camera_module_capture(void);

/* Returns a buffer obtained from camera_module_capture() to the driver. */
void camera_module_release(camera_fb_t *fb);

/* Wraps the captured RGB565 frame into ESP-DL's img_t struct WITHOUT COPYING
 * the data (it points at the same memory). The returned img_t is usable as
 * long as fb is valid; finish using it before fb is released. */
dl::image::img_t camera_fb_to_img(const camera_fb_t *fb);
