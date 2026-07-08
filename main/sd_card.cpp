#include "sd_card.h"
#include "esp_log.h"

static const char *TAG = "sd_card";
static bool s_mounted = false;

#if CONFIG_APP_ENABLE_SD_CARD

#include "pin_config.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

static sdmmc_card_t *s_card = nullptr;

esp_err_t sd_card_init(void)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 4;
    mount_config.allocation_unit_size = 16 * 1024;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = SD_SPI_MOSI_GPIO;
    bus_cfg.miso_io_num = SD_SPI_MISO_GPIO;
    bus_cfg.sclk_io_num = SD_SPI_SCK_GPIO;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;

    esp_err_t err = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "SPI bus baslatilamadi (0x%x); SD kart olmadan devam ediliyor.", err);
        return err;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_SPI_CS_GPIO;
    slot_config.host_id = (spi_host_device_t)host.slot;

    err = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &s_card);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "SD kart mount edilemedi (0x%x); erisim loglari sadece seri porta yazilacak.", err);
        return err;
    }

    s_mounted = true;
    ESP_LOGI(TAG, "SD kart hazir: /sdcard");
    return ESP_OK;
}

#else /* !CONFIG_APP_ENABLE_SD_CARD */

esp_err_t sd_card_init(void)
{
    ESP_LOGI(TAG, "SD kart destegi menuconfig'de kapali (APP_ENABLE_SD_CARD).");
    return ESP_ERR_NOT_SUPPORTED;
}

#endif

bool sd_card_is_mounted(void)
{
    return s_mounted;
}
