# ESP32-S3 AI CAM — Operatör Tanımalı Makine Yetkilendirme Sistemi

Meslek lisesi elektrik-elektronik atölyesi için eğitim amaçlı demo. Yüz
tanıma ile kayıtlı bir operatör doğrulanırsa bir GPIO üzerinden "çalışmaya
izin" sinyali üretir (röle/optokuplör → PLC girişi veya kontaktör bobini).

## ⚠️ GÜVENLİK / EMNİYET UYARISI (mutlaka okuyun)

1. **Bu sistemin ürettiği "izin" sinyali makineyi DOĞRUDAN çalıştırmaz.**
   Sadece makinenin fiziksel start butonunu "aktif" hale getirir. Operatör
   makineyi çalıştırmak için yine de fiziksel start butonuna basmalıdır.
   Röle çıkışını asla motor/kontaktör gücüne veya PLC'nin "run" hattına
   doğrudan bağlamayın.
2. **Bu sistemde canlılık tespiti (liveness) YOKTUR.** Bir fotoğrafla veya
   ekrandan gösterilen bir görüntüyle kandırılabilir. Gerçek/üretim bir
   yetkilendirme sisteminde mutlaka ikinci bir faktör eklenmelidir (örn.
   RFID kart okuma veya PIN kodu girişi, "VE" mantığıyla).
3. **Acil-stop butonu, kapı/guard switch'i gibi emniyet devreleri bu
   yazılımdan tamamen bağımsız olmalıdır.** Yazılım donarsa, hatalı "izin"
   üretirse veya çökerse/resetlenirse dahi bu fiziksel emniyetler makineyi
   durdurabilmelidir.

## Donanım

- Kart: ESP32-S3 AI CAM (OV3660 3MP kamera, 8MB PSRAM, 16MB Flash)
- 1x optokuplör/röle modülü (GPIO → PLC girişi / kontaktör bobini)
- 2x LED (yeşil: izin verildi, kırmızı: reddedildi) + 220-330Ω dirençler
- Opsiyonel: microSD kart modülü (SPI)

### Pin haritası (`main/pin_config.h`)

"ESP32-S3 AI CAM" adıyla satılan kartların tek bir resmi pinout'u yoktur;
aşağıdaki kamera pinleri yaygın klonlara göre **varsayımdır**. Kartınız
farklıysa **sadece `main/pin_config.h` dosyasını güncelleyin**, başka
hiçbir dosyaya dokunmanız gerekmez.

| Sinyal | GPIO |
|---|---|
| XCLK | 15 |
| SIOD/SDA | 4 |
| SIOC/SCL | 5 |
| D0..D7 | 11, 9, 8, 10, 12, 18, 17, 16 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |
| Röle çıkışı | 21 |
| Yeşil LED | 47 |
| Kırmızı LED | 48 |
| Kayıt (enroll) butonu | 0 (BOOT) |
| SD CS / MOSI / MISO / SCK (opsiyonel) | 39 / 40 / 41 / 42 |

### Bağlantı şeması (metin)

- **Kamera**: Karta lehimli/dahili, harici kablolama gerekmez.
- **Röle modülü**: `RELAY_OUTPUT_GPIO` (GPIO21) → röle modülünün IN pini.
  Röle modülünün VCC/GND'sini mümkünse ayrı bir 3.3V/5V kaynaktan besleyin
  (bobin akımı board regülatörünü zorlayabilir). Röle NO/COM kontakları
  PLC girişine veya kontaktör bobinine gider.
- **Yeşil LED**: GPIO47 → 220-330Ω direnç → LED anot; katot → GND.
- **Kırmızı LED**: GPIO48 → aynı şekilde.
- **SD kart (opsiyonel)**: SPI modu, CS=39, MOSI=40, MISO=41, SCK=42.

## Yazılım Mimarisi

```
main/
  pin_config.h        - Tüm pin tanımları (tek yer)
  app_main.cpp         - Başlatma sırası + ana döngü
  camera_module.*      - Kamera init + img_t dönüşümü
  face_engine.*         - HumanFaceDetect + HumanFaceFeat, kendi cosine-
                          similarity eşleştirmesi ve yapılandırılabilir eşik
  operator_store.*      - Operatör kayıtları (FATFS "facedb" partition)
  settings.*            - NVS: eşik ve izin süresi
  auth_output.*         - Röle + LED kontrolü, N sn sonra otomatik kapatma
  access_log.*          - Seri port + opsiyonel SD CSV logu
  sd_card.*             - Opsiyonel SPI SD mount
  serial_cli.*          - Komut satırı arayüzü
```

### Önemli tasarım notu: neden özel eşik mantığı?

esp-who'nun hazır `HumanFaceRecognizer` sınıfı benzerlik eşiğini (`0.5`)
kodun içinde sabitler ve değiştirmek için genel bir metot sunmaz. Bu
projede eşiğin yapılandırılabilir olması istendiği için `HumanFaceDetect`
+ `HumanFaceFeat` doğrudan kullanılıyor; karşılaştırma (cosine similarity)
ve eşik kontrolü `face_engine.cpp` içinde kendi kodumuzla yapılıyor. Bu
yaklaşımın bir faydası da öğrencilerin "benzerlik eşiği" kavramını kod
üzerinde birebir görebilmesi.

## Operatör Tanıtma (Enrollment)

İki yol var:

1. **Seri komutla (isim verilebildiği için önerilir):**
   ```
   operator-auth> enroll Ahmet
   ```
   Sistem 3 saniyelik geri sayım gösterir, operatör kameraya bakar, yüz
   tespit edilip özniteliği çıkarılır ve kaydedilir.

2. **BOOT tuşuyla (isim giremediği için otomatik isim):** Kart çalışırken
   BOOT tuşuna kısa basış aynı akışı `operator_N` gibi otomatik bir isimle
   başlatır. Sonradan `list` ile hangi id'nin kime ait olduğunu
   eşleştirebilirsiniz.

### Diğer komutlar

| Komut | Açıklama |
|---|---|
| `list` | Kayıtlı operatörleri id/isim/kayıt zamanıyla listeler |
| `delete <id>` | Operatörü siler |
| `set-threshold <0..1>` | Benzerlik eşiğini ayarlar (varsayılan `0.55`) |
| `set-duration <sn>` | İzin süresini ayarlar (varsayılan `5`) |
| `status` | Güncel eşik, süre ve operatör sayısını gösterir |

En fazla **10 operatör** desteklenir. `enroll` komutu doluysa reddedilir.

## Kurulum

### 1. ESP-IDF v5.2 kurulumu (Windows)

- [ESP-IDF Windows kurulum kılavuzunu](https://docs.espressif.com/projects/esp-idf/en/v5.2/esp32s3/get-started/windows-setup.html)
  izleyerek ESP-IDF v5.2'yi kurun (VS Code "ESP-IDF" eklentisi ile veya
  "ESP-IDF Tools" kurulumuyla).
- VS Code'da `ESP-IDF: Configure ESP-IDF Extension` ile v5.2 ortamını seçin.

### 2. Projeyi açma ve hedef seçme

```
idf.py set-target esp32s3
```

Bu adım `idf_component.yml` içindeki bağımlılıkları (esp32-camera,
human_face_detect, human_face_recognition → bunlar esp-dl'i otomatik
çeker) internetten indirir.

### 3. menuconfig kontrolü

`sdkconfig.defaults` gerekli ayarları (PSRAM Octal/80MHz, özel partition
tablosu, USB-Serial/JTAG konsol, C++ exceptions, model saklama yeri)
zaten içerir. Yine de doğrulamak isterseniz:

```
idf.py menuconfig
```

- `Component config → ESP PSRAM` → Octal Mode, 80MHz açık olmalı.
- `Serial flasher config → Flash size` → 16 MB.
- `Component config → human_face_detect` / `human_face_recognition` →
  model varyantı (ESPDET_PICO_224_224_FACE / MFN_S8_V1) seçili olmalı.
- SD kart kullanacaksanız: `Operator Yetkilendirme Uygulaması →
  SD karta erişim logu yaz` seçeneğini açın.

### 4. Derleme, yükleme, izleme

```
idf.py build
idf.py -p COMx flash monitor
```

(`COMx` yerine kartın bağlı olduğu seri portu yazın; Aygıt Yöneticisi'nden
kontrol edebilirsiniz.)

### Derlerken karşılaşabileceğiniz noktalar

- **`HumanFaceFeat`/`TensorBase` veri tipi**: `face_engine.cpp` içindeki
  `tensor_to_float()` fonksiyonu modelin çıktısının `float` ya da `int8`
  (nicelenmiş) olmasına göre iki yol izler. esp-dl sürümünüzde farklı bir
  tip dönerse derleyici/çalışma zamanı uyarısı verir; bu fonksiyonu o
  sürümün `dl_tensor_base.hpp` tanımına göre güncelleyin.
- **Öznitelik vektörü uzunluğu**: `operator_store.h` içindeki
  `OPERATOR_FEAT_LEN` (512) `HumanFaceFeat::get_feat_len()` ile
  karşılaştırılır (`face_engine_init()` içinde); uyuşmazsa net bir hata
  logu basar ve başlatmayı durdurur.
- **Kamera pin/format uyumu**: OV3660 farklı bir klonda farklı XCLK
  frekansı gerektirebilir; görüntü bozuksa `CAM_XCLK_FREQ_HZ` değerini
  (`pin_config.h`) 24000000 gibi bir değerle deneyin.
