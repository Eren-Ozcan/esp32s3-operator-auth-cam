# TODO — Eksikler ve Yapılacaklar

Bu proje hiç derlenmedi/flaşlanmadı (bu ortamda ESP-IDF toolchain yok).
Kod, GitHub'dan doğrulanmış gerçek API imzalarına göre yazıldı ama gerçek
donanımda hiç test edilmedi. Aşağıdaki liste, sınıfta güvenle kullanmadan
önce yapılması gerekenleri önceliğe göre sıralar.

## 1) Derleme / ilk flash (mutlaka yapılmalı)

- [ ] `idf.py set-target esp32s3` + `idf.py build` çalıştırıp ilk derleme
      hatalarını gider. En olası 3 nokta zaten kodda/README'de işaretli:
  - [ ] `face_engine.cpp` → `tensor_to_float()`: model çıktısı `float` mı
        `int8` mi, gerçek esp-dl sürümünde doğrula.
  - [ ] `operator_store.h` → `OPERATOR_FEAT_LEN` (512 varsayıldı);
        `face_engine_init()` bunu çalışma zamanında karşılaştırıp hata
        basıyor, uyuşmazsa güncelle.
  - [ ] `idf_component.yml`'deki `espressif/esp32-camera`,
        `espressif/human_face_detect`, `espressif/human_face_recognition`
        sürüm aralıklarının component registry'de hâlâ geçerli olduğunu
        doğrula (zamanla yeni majör sürümler API kırabilir).
- [ ] Kartı flaşlayıp seri monitörde açılış loglarının hatasız geçtiğini
      doğrula (kamera init, model yükleme, NVS, facedb mount).

## 2) Donanım bring-up (kart elinize geçince)

- [ ] `main/pin_config.h`'deki kamera pinlerini kartın gerçek şemasıyla
      karşılaştır — bu değerler varsayımdı, doğrulanmadı.
- [ ] Görüntü ters/bozuk geliyorsa `CAM_XCLK_FREQ_HZ` değerini (20MHz)
      24MHz gibi bir değerle dene.
- [ ] Röle/optokuplör modülünü **PLC'ye bağlamadan önce** sadece LED/multimetre
      ile test et (GPIO21 çıkışının doğru seviyede anahtarladığını doğrula).
- [ ] Röle modülünün beslemesini board'un 3.3V hattından değil, ayrı bir
      kaynaktan verdiğini kontrol et (README'de zaten uyarılmıştı).
- [ ] `ENROLL_BUTTON_GPIO` (BOOT/GPIO0) kart çalışırken gerçekten
      kullanılabiliyor mu, yoksa ayrı bir fiziksel buton mu eklenecek, karar ver.
- [ ] SD kart özelliğini kullanacaksan `APP_ENABLE_SD_CARD` açıp SPI
      pinlerinin (39/40/41/42) kartta boş/uygun olduğunu doğrula.

## 3) Kalibrasyon (sınıf ortamında)

- [ ] Varsayılan eşik (`0.55`) sınıfın ışık koşullarında test edilip
      `set-threshold` ile ayarlanmalı — yanlış red / yanlış kabul oranını
      birkaç öğrenciyle deneyerek bul.
- [ ] Varsayılan izin süresi (5 sn) demo senaryosuna göre `set-duration`
      ile ayarlanmalı.
- [ ] En az 3-5 farklı kişiyle enroll/recognize denemesi yapılıp gözle
      görülür bir yanlış-pozitif/negatif var mı kontrol edilmeli.

## 4) Bilinen eksikler / yapılmamış özellikler

- [ ] **Toplu silme yok**: sadece `delete <id>` var, tüm kayıtları tek
      komutla temizleyen bir `clear-all` yok (facedb'yi manuel silmeden
      sıfırlamak için faydalı olurdu).
- [ ] **Enroll iptali yok**: `enroll` başlatıldıktan sonra 3 saniyelik geri
      sayımı iptal eden bir komut/tuş yok.
- [ ] **SD log dosyası büyümesi sınırlanmıyor**: `/sdcard/access_log.csv`
      hiç döndürülmüyor/boyut sınırı yok; uzun süre çalışan bir demo
      kartında dosya büyüyebilir.
- [ ] **Kamera/model init hatası → boot loop**: `app_main.cpp` içinde
      `ESP_ERROR_CHECK` kullanıldığı için kamera veya model yüklenemezse
      cihaz sürekli resetlenir; sınıf ortamında "neden sürekli reset
      atıyor" karışıklığına yol açabilir. Daha yumuşak bir hata ekranı/log
      döngüsü eklenebilir.
- [ ] **Buton debounce'u kaba**: `enroll_button_pressed_edge()` sadece ana
      döngü periyoduna (~150ms) güveniyor; donanımsal debounce veya daha
      sıkı yazılımsal debounce eklenebilir.

## 5) İyileştirme fikirleri (opsiyonel, zorunlu değil)

- [ ] Röle çıkışına paralel bir "test modu" (LED'lerle simülasyon,
      röleyi hiç bağlamadan yazılımı göstermek için) eklenebilir.
- [ ] `status` komutuna son 5 tanıma denemesinin özetini ekleme.
- [ ] Öğrencilere gösterim için basit bir bağlantı şeması görseli
      (Fritzing/KiCad) — şu an sadece metin açıklaması var.
- [ ] BOM (malzeme listesi: röle modeli, LED, direnç değerleri) ekleme.
