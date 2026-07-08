#include "auth_output.h"
#include "pin_config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "auth_output";
static esp_timer_handle_t s_grant_timer = nullptr;

static void grant_timer_cb(void *arg)
{
    ESP_LOGI(TAG, "Izin suresi doldu, rolo cikisi kapatiliyor.");
    gpio_set_level(RELAY_OUTPUT_GPIO, 0);
    gpio_set_level(LED_GREEN_GPIO, 0);
}

esp_err_t auth_output_init(void)
{
    gpio_config_t conf = {};
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = (1ULL << RELAY_OUTPUT_GPIO) | (1ULL << LED_GREEN_GPIO) | (1ULL << LED_RED_GPIO);
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    esp_err_t err = gpio_config(&conf);
    if (err != ESP_OK) {
        return err;
    }

    const esp_timer_create_args_t timer_args = {
        .callback = &grant_timer_cb,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "grant_timer",
    };
    err = esp_timer_create(&timer_args, &s_grant_timer);
    if (err != ESP_OK) {
        return err;
    }

    auth_output_idle();
    return ESP_OK;
}

void auth_output_grant(uint32_t seconds)
{
    ESP_LOGI(TAG, "IZIN VERILDI - rolo cikisi %u sn acik kalacak.", (unsigned)seconds);
    gpio_set_level(RELAY_OUTPUT_GPIO, 1);
    gpio_set_level(LED_GREEN_GPIO, 1);
    gpio_set_level(LED_RED_GPIO, 0);

    /* Onceki zamanlayici hala calisiyorsa iptal edip yeniden baslat
     * (art arda basarili tanimalarda sure her seferinde yenilenir). */
    esp_timer_stop(s_grant_timer);
    esp_timer_start_once(s_grant_timer, (uint64_t)seconds * 1000000ULL);
}

void auth_output_deny(void)
{
    gpio_set_level(RELAY_OUTPUT_GPIO, 0);
    gpio_set_level(LED_GREEN_GPIO, 0);
    gpio_set_level(LED_RED_GPIO, 1);
}

void auth_output_idle(void)
{
    gpio_set_level(RELAY_OUTPUT_GPIO, 0);
    gpio_set_level(LED_GREEN_GPIO, 0);
    gpio_set_level(LED_RED_GPIO, 0);
}
