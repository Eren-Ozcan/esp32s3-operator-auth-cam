#include "serial_cli.h"
#include "operator_store.h"
#include "settings.h"
#include "esp_console.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
#include <cstring>
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "serial_cli";

static SemaphoreHandle_t s_lock = nullptr;
static bool s_enroll_pending = false;
static char s_enroll_name[OPERATOR_NAME_LEN] = {0};

void serial_cli_request_enroll(const char *name)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_enroll_pending) {
        ESP_LOGW(TAG, "Zaten bekleyen bir kayit istegi var, yeni istek yoksayildi.");
    } else {
        s_enroll_pending = true;
        strncpy(s_enroll_name, name, OPERATOR_NAME_LEN - 1);
        s_enroll_name[OPERATOR_NAME_LEN - 1] = '\0';
        printf("Kayit istegi alindi (isim: %s). Kameraya bakin, 3 saniye icinde islenecek...\n", s_enroll_name);
    }
    xSemaphoreGive(s_lock);
}

bool serial_cli_take_enroll_request(char *name_out, size_t max_len)
{
    bool taken = false;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_enroll_pending) {
        strncpy(name_out, s_enroll_name, max_len - 1);
        name_out[max_len - 1] = '\0';
        s_enroll_pending = false;
        taken = true;
    }
    xSemaphoreGive(s_lock);
    return taken;
}

/* --- enroll <isim> --- */
static struct {
    struct arg_str *name;
    struct arg_end *end;
} s_enroll_args;

static int cmd_enroll(int argc, char **argv)
{
    int errors = arg_parse(argc, argv, (void **)&s_enroll_args);
    if (errors != 0) {
        arg_print_errors(stderr, s_enroll_args.end, argv[0]);
        return 1;
    }
    serial_cli_request_enroll(s_enroll_args.name->sval[0]);
    return 0;
}

/* --- delete <id> --- */
static struct {
    struct arg_int *id;
    struct arg_end *end;
} s_delete_args;

static int cmd_delete(int argc, char **argv)
{
    int errors = arg_parse(argc, argv, (void **)&s_delete_args);
    if (errors != 0) {
        arg_print_errors(stderr, s_delete_args.end, argv[0]);
        return 1;
    }
    int id = s_delete_args.id->ival[0];
    esp_err_t err = operator_store_delete((uint8_t)id);
    if (err == ESP_OK) {
        printf("id=%d silindi.\n", id);
    } else {
        printf("Silinemedi: id=%d bulunamadi.\n", id);
    }
    return 0;
}

/* --- list --- */
static int cmd_list(int argc, char **argv)
{
    int count = operator_store_count();
    printf("Kayitli operator sayisi: %d / %d\n", count, OPERATOR_MAX_COUNT);
    for (int i = 0; i < count; i++) {
        const operator_record_t *rec = operator_store_get_by_index(i);
        printf("  id=%d  isim=%s  kayit_zamani(s)=%u\n", rec->id, rec->name, (unsigned)rec->enrolled_at_s);
    }
    return 0;
}

/* --- set-threshold <deger> --- */
static struct {
    struct arg_dbl *value;
    struct arg_end *end;
} s_threshold_args;

static int cmd_set_threshold(int argc, char **argv)
{
    int errors = arg_parse(argc, argv, (void **)&s_threshold_args);
    if (errors != 0) {
        arg_print_errors(stderr, s_threshold_args.end, argv[0]);
        return 1;
    }
    float value = (float)s_threshold_args.value->dval[0];
    esp_err_t err = settings_set_threshold(value);
    if (err == ESP_OK) {
        printf("Esik degeri %.2f olarak ayarlandi.\n", value);
    } else {
        printf("Gecersiz deger; 0.0-1.0 arasinda olmali.\n");
    }
    return 0;
}

/* --- set-duration <saniye> --- */
static struct {
    struct arg_int *value;
    struct arg_end *end;
} s_duration_args;

static int cmd_set_duration(int argc, char **argv)
{
    int errors = arg_parse(argc, argv, (void **)&s_duration_args);
    if (errors != 0) {
        arg_print_errors(stderr, s_duration_args.end, argv[0]);
        return 1;
    }
    int seconds = s_duration_args.value->ival[0];
    esp_err_t err = settings_set_grant_seconds((uint32_t)seconds);
    if (err == ESP_OK) {
        printf("Izin suresi %d saniye olarak ayarlandi.\n", seconds);
    } else {
        printf("Gecersiz deger; 1 veya daha buyuk olmali.\n");
    }
    return 0;
}

/* --- status --- */
static int cmd_status(int argc, char **argv)
{
    printf("Esik: %.2f\n", settings_get_threshold());
    printf("Izin suresi: %u sn\n", (unsigned)settings_get_grant_seconds());
    printf("Kayitli operator: %d / %d\n", operator_store_count(), OPERATOR_MAX_COUNT);
    return 0;
}

esp_err_t serial_cli_init(void)
{
    s_lock = xSemaphoreCreateMutex();

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "operator-auth>";
    repl_config.max_cmdline_length = 256;

    esp_console_dev_usb_serial_jtag_config_t jtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    esp_err_t err = esp_console_new_repl_usb_serial_jtag(&jtag_config, &repl_config, &repl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Konsol baslatilamadi: 0x%x", err);
        return err;
    }

    s_enroll_args.name = arg_str1(NULL, NULL, "<isim>", "Kaydedilecek operatorun ismi");
    s_enroll_args.end = arg_end(1);
    const esp_console_cmd_t enroll_cmd = {
        .command = "enroll",
        .help = "Kameraya bakan kisiyi verilen isimle kaydeder",
        .hint = NULL,
        .func = &cmd_enroll,
        .argtable = &s_enroll_args,
    };
    esp_console_cmd_register(&enroll_cmd);

    s_delete_args.id = arg_int1(NULL, NULL, "<id>", "Silinecek operator id'si");
    s_delete_args.end = arg_end(1);
    const esp_console_cmd_t delete_cmd = {
        .command = "delete",
        .help = "Verilen id'ye sahip operatoru siler",
        .hint = NULL,
        .func = &cmd_delete,
        .argtable = &s_delete_args,
    };
    esp_console_cmd_register(&delete_cmd);

    const esp_console_cmd_t list_cmd = {
        .command = "list",
        .help = "Kayitli operatorleri listeler",
        .hint = NULL,
        .func = &cmd_list,
    };
    esp_console_cmd_register(&list_cmd);

    s_threshold_args.value = arg_dbl1(NULL, NULL, "<0..1>", "Benzerlik esigi");
    s_threshold_args.end = arg_end(1);
    const esp_console_cmd_t threshold_cmd = {
        .command = "set-threshold",
        .help = "Yuz tanima benzerlik esigini ayarlar (0.0-1.0)",
        .hint = NULL,
        .func = &cmd_set_threshold,
        .argtable = &s_threshold_args,
    };
    esp_console_cmd_register(&threshold_cmd);

    s_duration_args.value = arg_int1(NULL, NULL, "<sn>", "Izin suresi (saniye)");
    s_duration_args.end = arg_end(1);
    const esp_console_cmd_t duration_cmd = {
        .command = "set-duration",
        .help = "Rolo cikisinin acik kalacagi sureyi ayarlar (saniye)",
        .hint = NULL,
        .func = &cmd_set_duration,
        .argtable = &s_duration_args,
    };
    esp_console_cmd_register(&duration_cmd);

    const esp_console_cmd_t status_cmd = {
        .command = "status",
        .help = "Guncel esik, sure ve operator sayisini gosterir",
        .hint = NULL,
        .func = &cmd_status,
    };
    esp_console_cmd_register(&status_cmd);

    err = esp_console_start_repl(repl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Konsol REPL baslatilamadi: 0x%x", err);
        return err;
    }

    ESP_LOGI(TAG, "Seri komut arayuzu hazir. Komutlar: enroll, delete, list, set-threshold, set-duration, status");
    return ESP_OK;
}
