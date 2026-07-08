#include "camera_module.h"
#include "pin_config.h"
#include "esp_log.h"

static const char *TAG = "camera_module";

esp_err_t camera_module_init(void)
{
    camera_config_t config = {};
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.pin_xclk = CAM_PIN_XCLK;
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;

    config.pin_d7 = CAM_PIN_D7;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d0 = CAM_PIN_D0;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_pclk = CAM_PIN_PCLK;

    config.xclk_freq_hz = CAM_XCLK_FREQ_HZ;
    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;

    /* RGB565: yuz tespit/tanima modelleri dogrudan piksel matrisi ister,
     * JPEG cozme adimindan kacinip gecikmeyi dusurur. */
    config.pixel_format = PIXFORMAT_RGB565;
    /* 240x240 civari (FRAMESIZE_240X240) hem yuz tespiti icin yeterli
     * cozunurluk saglar hem de PSRAM/isleme yukunu makul tutar. */
    config.frame_size = FRAMESIZE_240X240;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Kamera baslatilamadi: 0x%x", err);
        return err;
    }

    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor != NULL && sensor->id.PID == OV3660_PID) {
        /* OV3660'in varsayilan goruntusu ters ve fazla doygun renklidir. */
        sensor->set_vflip(sensor, 1);
        sensor->set_brightness(sensor, 1);
        sensor->set_saturation(sensor, -2);
    }

    ESP_LOGI(TAG, "Kamera hazir (RGB565, %dx%d)", 240, 240);
    return ESP_OK;
}

camera_fb_t *camera_module_capture(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb == NULL) {
        ESP_LOGW(TAG, "Kare yakalanamadi");
    }
    return fb;
}

void camera_module_release(camera_fb_t *fb)
{
    if (fb != NULL) {
        esp_camera_fb_return(fb);
    }
}

dl::image::img_t camera_fb_to_img(const camera_fb_t *fb)
{
    dl::image::img_t img = {};
    img.data = (void *)fb->buf;
    img.width = fb->width;
    img.height = fb->height;
    /* esp32-camera RGB565 ciktisini little-endian byte sirasiyla verir. */
    img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB565LE;
    return img;
}
