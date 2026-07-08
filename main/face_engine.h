/*
 * face_engine.h
 * -----------------------------------------------------------------------
 * ESP-DL'in HumanFaceDetect (yuz tespiti) ve HumanFaceFeat (oznitelik
 * cikarma) siniflarini dogrudan kullanir.
 *
 * ONEMLI TASARIM NOTU: esp-who'nun hazir HumanFaceRecognizer sinifi
 * (esp-dl/models/human_face_recognition) benzerlik esigini (m_thr)
 * constructor'da 0.5'e sabitler ve degistirmek icin public bir metot
 * SUNMAZ. Bu projede "eşik yapılandırılabilir olsun" gereksinimi oldugu
 * icin HumanFaceRecognizer atlanmis; bunun yerine HumanFaceFeat ile
 * oznitelik vektoru cikarilip, karsilastirma (cosine similarity) ve esik
 * kontrolu burada, kendi kodumuzda yapilmaktadir. Boylece ogrenciler
 * "benzerlik esigi" kavramini kod uzerinde somut olarak gorur.
 */
#pragma once

#include "esp_err.h"
#include "esp_camera.h"
#include "operator_store.h"

struct face_identify_result_t {
    bool face_found;      /* Karede en az bir yuz tespit edildi mi */
    bool matched;         /* Esik ustu bir eslesme bulundu mu */
    uint8_t operator_id;   /* matched == true ise gecerlidir */
    float similarity;      /* En iyi adayin benzerlik skoru (0..1) */
};

esp_err_t face_engine_init(void);

/* Karedeki en buyuk (kameraya en yakin) yuzu kayitli operatorlerle
 * karsilastirir. Yuz yoksa face_found=false doner. */
face_identify_result_t face_engine_identify(const camera_fb_t *fb);

/* Karedeki en buyuk yuzun oznitelik vektorunu cikarir (enrollment icin).
 * out_feat en az OPERATOR_FEAT_LEN eleman icin ayrilmis olmalidir.
 * Yuz bulunamazsa false doner. */
bool face_engine_extract_feat(const camera_fb_t *fb, float *out_feat, int feat_len);
