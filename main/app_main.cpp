/*
 * app_main.cpp
 * -----------------------------------------------------------------------
 * Entry point: initializes all modules in order, then runs the continuous
 * "capture -> detect -> compare -> drive output" loop.
 *
 * This file is FOR TEACHING; it is deliberately kept simple (a single
 * FreeRTOS task) so that the whole flow can be followed step by step in one
 * place.
 */
#include "pin_config.h"
#include "camera_module.h"
#include "face_engine.h"
#include "operator_store.h"
#include "settings.h"
#include "auth_output.h"
#include "access_log.h"
#include "sd_card.h"
#include "serial_cli.h"

#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app_main";

static void enroll_button_init(void)
{
    gpio_config_t conf = {};
    conf.mode = GPIO_MODE_INPUT;
    conf.pin_bit_mask = (1ULL << ENROLL_BUTTON_GPIO);
    conf.pull_up_en = GPIO_PULLUP_ENABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&conf);
}

/* Detects a BOOT button press (edge triggered, simple debounce). Since no
 * name can be typed, it leaves an enrollment request with an automatic
 * "operator_N" name; for a custom name use "enroll <name>" over serial. */
static bool enroll_button_pressed_edge(void)
{
    static bool was_pressed = false;
    int level = gpio_get_level(ENROLL_BUTTON_GPIO);
    bool pressed_now = ENROLL_BUTTON_ACTIVE_LOW ? (level == 0) : (level == 1);

    bool edge = pressed_now && !was_pressed;
    was_pressed = pressed_now;
    return edge;
}

/* Enrollment flow: 3-second countdown -> grab a frame -> extract features ->
 * add to operator_store. Called from the main loop so that camera access
 * happens from a single place (this loop task). */
static void perform_enrollment(const char *name)
{
    printf("\n>>> Kayit basliyor: '%s'. Kameraya bakin. <<<\n", name);
    for (int remaining = 3; remaining > 0; remaining--) {
        printf("  %d...\n", remaining);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    camera_fb_t *fb = camera_module_capture();
    if (fb == NULL) {
        printf("Kayit basarisiz: kare alinamadi.\n");
        return;
    }

    float feat[OPERATOR_FEAT_LEN];
    bool face_found = face_engine_extract_feat(fb, feat, OPERATOR_FEAT_LEN);
    camera_module_release(fb);

    if (!face_found) {
        printf("Kayit basarisiz: yuzu tespit edemedim, tekrar deneyin.\n");
        return;
    }

    uint8_t new_id = 0;
    esp_err_t err = operator_store_add(name, feat, OPERATOR_FEAT_LEN, &new_id);
    if (err == ESP_OK) {
        printf("Basarili: id=%d isim=%s kaydedildi.\n\n", new_id, name);
    } else if (err == ESP_ERR_NO_MEM) {
        printf("Kayit basarisiz: maksimum operator sayisina (%d) ulasildi. Once 'delete' ile yer acin.\n\n",
               OPERATOR_MAX_COUNT);
    } else {
        printf("Kayit basarisiz: hata 0x%x\n\n", err);
    }
}

static void main_loop(void)
{
    char pending_name[OPERATOR_NAME_LEN];

    while (true) {
        if (enroll_button_pressed_edge()) {
            char auto_name[OPERATOR_NAME_LEN];
            snprintf(auto_name, sizeof(auto_name), "operator_%d", operator_store_count());
            serial_cli_request_enroll(auto_name);
        }

        if (serial_cli_take_enroll_request(pending_name, sizeof(pending_name))) {
            perform_enrollment(pending_name);
            continue; /* bu iterasyonda tanima yapma, bir sonrakinde devam et */
        }

        camera_fb_t *fb = camera_module_capture();
        if (fb == NULL) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        face_identify_result_t result = face_engine_identify(fb);
        camera_module_release(fb);

        if (result.face_found) {
            if (result.matched) {
                const operator_record_t *rec = operator_store_find(result.operator_id);
                auth_output_grant(settings_get_grant_seconds());
                access_log_record(true, result.operator_id, rec != NULL ? rec->name : "?", result.similarity);
            } else {
                auth_output_deny();
                access_log_record(false, -1, "bilinmiyor", result.similarity);
            }
        }
        /* When no face is detected nothing is logged (to avoid spamming on
         * empty frames) and the current relay/LED state is left alone. */

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(settings_init());
    ESP_ERROR_CHECK(operator_store_init());
    ESP_ERROR_CHECK(camera_module_init());
    ESP_ERROR_CHECK(face_engine_init());
    ESP_ERROR_CHECK(auth_output_init());

    sd_card_init(); /* opsiyonel; basarisiz olsa da sistem calismaya devam eder */
    access_log_init();

    enroll_button_init();
    ESP_ERROR_CHECK(serial_cli_init());

    ESP_LOGI(TAG, "Sistem hazir. 'enroll <isim>' ile operator kaydedebilir, 'status' ile durumu gorebilirsiniz.");

    main_loop();
}
