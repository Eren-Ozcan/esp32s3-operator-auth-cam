/*
 * pin_config.h
 * -----------------------------------------------------------------------
 * TUM GPIO PIN TANIMLARI BU DOSYADA TOPLANMISTIR.
 *
 * "ESP32-S3 AI CAM" etiketiyle satilan kartlarin tek bir resmi/standart
 * pin yerlesimi YOKTUR (klon karta gore degisebilir). Asagidaki kamera
 * pinleri bu tip kartlarda en sik goruelen (Freenove / genel ESP32-S3-CAM
 * tarzi) yerlesime gore VARSAYIM olarak secilmistir.
 *
 * >>> Kartinizin serigrafisi/semasi farkliysa SADECE bu dosyayi guncelleyin. <<<
 *
 * -----------------------------------------------------------------------
 * GUVENLIK / EMNIYET UYARISI (ozet - detay README.md ve auth_output.h'de):
 *   - Bu sistemin urettigi "izin" sinyali bir makineyi DOGRUDAN calistirmaz;
 *     sadece fiziksel start butonunu "aktif" hale getirir. Operator yine
 *     fiziksel butona basmalidir.
 *   - Bu sistemde CANLILIK TESPITI (liveness) YOKTUR. Bir fotografla
 *     kandirilabilir. Gercek/uretim bir yetkilendirme sisteminde ikinci
 *     faktor (RFID kart, PIN kodu vb.) MUTLAKA eklenmelidir.
 *   - Acil-stop, kapi switch'i gibi elektromekanik emniyet devreleri bu
 *     yazilimdan TAMAMEN BAGIMSIZ olmalidir. Yazilim donsa, hatali "izin"
 *     verse veya cokse bile bu fiziksel emniyet devreleri makineyi
 *     durdurabilmelidir.
 * -----------------------------------------------------------------------
 */
#pragma once

#include "driver/gpio.h"

/* ======================= KAMERA PINLERI (OV3660, dahili) ======================= */
/* Kamera modulu karta lehimli geldigi icin bu pinler harici kablolama
 * gerektirmez; sadece dogru sabit degerlerin secilmesi gerekir. */
#define CAM_PIN_PWDN    (-1)   /* Cogu klonda bagli degil */
#define CAM_PIN_RESET   (-1)   /* Cogu klonda bagli degil */
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4      /* SCCB/I2C SDA */
#define CAM_PIN_SIOC    5      /* SCCB/I2C SCL */

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11

#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

#define CAM_XCLK_FREQ_HZ  20000000

/* ======================= YETKILENDIRME CIKISI ======================= */
/* Optokuplor/rolo modulunun IN pinine baglanir. Rolo cikisi (NO/COM)
 * PLC girisine veya kontaktor bobinine gider - bkz. README baglanti semasi. */
#define RELAY_OUTPUT_GPIO   GPIO_NUM_21

/* Durum LED'leri (220-330 ohm dirençle GPIO -> LED anot, katot -> GND) */
#define LED_GREEN_GPIO       GPIO_NUM_47   /* Izin verildi */
#define LED_RED_GPIO         GPIO_NUM_48   /* Reddedildi / beklemede */

/* ======================= KAYIT (ENROLLMENT) BUTONU ======================= */
/* Cogu AI-CAM klon kartinda BOOT tusu disinda kullanicIya ayrilmis ayri bir
 * buton bulunmaz. BOOT (GPIO0) acilistan SONRA normal bir girdi pini gibi
 * kullanilabilir (yalniz acilis/reset anindaki islevini karistirmayin: kart
 * calisirken kisa basis sorunsuzdur). Kartinizda ayri bir buton varsa bu
 * degeri degistirin. */
#define ENROLL_BUTTON_GPIO   GPIO_NUM_0
#define ENROLL_BUTTON_ACTIVE_LOW  1

/* ======================= SD KART (OPSIYONEL, SPI MODU) ======================= */
/* Sadece CONFIG_APP_ENABLE_SD_CARD acikken kullanilir (bkz. Kconfig.projbuild). */
#define SD_SPI_CS_GPIO     GPIO_NUM_39
#define SD_SPI_MOSI_GPIO   GPIO_NUM_40
#define SD_SPI_MISO_GPIO   GPIO_NUM_41
#define SD_SPI_SCK_GPIO    GPIO_NUM_42
