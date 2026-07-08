/*
 * auth_output.h
 * -----------------------------------------------------------------------
 * Rolo/optokuplor cikisini ve durum LED'lerini kontrol eder.
 *
 * ############################################################################
 * # GUVENLIK / EMNIYET UYARISI - MUTLAKA OKUYUN                             #
 * #                                                                          #
 * # 1) Bu modulun urettigi "izin" sinyali bir makineyi DOGRUDAN CALISTIRMAZ. #
 * #    Sadece makinenin fiziksel start butonunu "aktif" hale getirir.        #
 * #    Operator, makineyi calistirmak icin YINE DE fiziksel start butonuna   #
 * #    basmalidir. Rolo cikisini asla motor/kontaktor gucune veya PLC'nin    #
 * #    "run" hattina dogrudan baglamayin.                                    #
 * #                                                                          #
 * # 2) Bu sistemde CANLILIK TESPITI (liveness) YOKTUR. Bir fotografla veya   #
 * #    ekrandan gosterilen bir goruntuyle kandirilabilir. Gercek/uretim bir  #
 * #    yetkilendirme sisteminde MUTLAKA ikinci bir faktor eklenmelidir       #
 * #    (orn. RFID kart okuma veya PIN kodu girisi ile "VE" mantigi).         #
 * #                                                                          #
 * # 3) Acil-stop butonu, kapi/guard switch'i gibi elektromekanik emniyet     #
 * #    devreleri bu yazilimdan TAMAMEN BAGIMSIZ, dogrudan enerjiyi kesen     #
 * #    ayri devreler olarak tasarlanmalidir. Yazilim donarsa, hatali "izin"  #
 * #    uretirse veya coker/resetlenirse dahi bu fiziksel emniyetler          #
 * #    makineyi durdurabilmelidir. Yazilimin hicbir cikisi bu devrelerin     #
 * #    yerini ALAMAZ.                                                        #
 * ############################################################################
 */
#pragma once

#include "esp_err.h"

esp_err_t auth_output_init(void);

/* Rolo cikisini HIGH yapar, yesil LED yakar, kirmizi LED'i sondurur ve
 * `seconds` saniye sonra otomatik olarak LOW'a donmesini planlar
 * (esp_timer ile - engelleyici degildir, cagiran task blocke olmaz). */
void auth_output_grant(uint32_t seconds);

/* Rolo cikisini LOW tutar, kirmizi LED'i kisa sure yakar (reddedildi
 * gostergesi), yesil LED'i sondurur. */
void auth_output_deny(void);

/* Baslangic / bekleme durumu: rolo LOW, her iki LED sonuk. */
void auth_output_idle(void);
