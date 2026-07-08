#include "face_engine.h"
#include "camera_module.h"
#include "settings.h"
#include "human_face_detect.hpp"
#include "human_face_recognition.hpp"
#include "dl_define.hpp"
#include "dl_tensor_base.hpp"
#include "esp_log.h"
#include <algorithm>
#include <cmath>

static const char *TAG = "face_engine";

static HumanFaceDetect *s_detect = nullptr;
static HumanFaceFeat *s_feat = nullptr;

esp_err_t face_engine_init(void)
{
    /* lazy_load=false: modelleri baslangicta yukle, ilk tanima aninda
     * beklenmedik gecikme olmasin. */
    s_detect = new HumanFaceDetect(HumanFaceDetect::ESPDET_PICO_224_224_FACE, false);
    s_feat = new HumanFaceFeat(HumanFaceFeat::MFN_S8_V1, false);

    int feat_len = s_feat->get_feat_len();
    if (feat_len != OPERATOR_FEAT_LEN) {
        ESP_LOGE(TAG,
                 "Model oznitelik uzunlugu (%d) operator_store.h icindeki "
                 "OPERATOR_FEAT_LEN (%d) ile uyusmuyor! O degeri guncelleyip "
                 "yeniden derleyin.",
                 feat_len, OPERATOR_FEAT_LEN);
        return ESP_ERR_INVALID_SIZE;
    }

    ESP_LOGI(TAG, "Yuz tespit/tanima modelleri yuklendi (feat_len=%d).", feat_len);
    return ESP_OK;
}

static const dl::detect::result_t *pick_largest_face(const std::list<dl::detect::result_t> &results)
{
    if (results.empty()) {
        return nullptr;
    }
    auto it = std::max_element(
        results.begin(), results.end(),
        [](const dl::detect::result_t &a, const dl::detect::result_t &b) { return a.box_area() < b.box_area(); });
    return &(*it);
}

/* TensorBase'in ham verisini float diziye cevirir. Model ciktisi float ise
 * dogrudan kopyalanir; int8 (nicelenmis) ise DL_SCALE(exponent) ile
 * dogru olcege cevrilir (bkz. esp-dl dl_define.hpp). */
static bool tensor_to_float(dl::TensorBase *tensor, float *out, int max_len)
{
    if (tensor == nullptr) {
        return false;
    }
    int len = tensor->get_size();
    if (len > max_len) {
        ESP_LOGE(TAG, "Oznitelik vektoru beklenenden uzun: %d > %d", len, max_len);
        return false;
    }

    if (tensor->get_dtype() == dl::DATA_TYPE_FLOAT) {
        float *src = tensor->get_element_ptr<float>();
        for (int i = 0; i < len; i++) {
            out[i] = src[i];
        }
    } else if (tensor->get_dtype() == dl::DATA_TYPE_INT8) {
        int8_t *src = tensor->get_element_ptr<int8_t>();
        float scale = DL_SCALE((int)tensor->exponent);
        for (int i = 0; i < len; i++) {
            out[i] = (float)src[i] * scale;
        }
    } else {
        ESP_LOGE(TAG, "Desteklenmeyen oznitelik veri tipi: %s", tensor->get_dtype_string());
        return false;
    }
    return true;
}

static float cosine_similarity(const float *a, const float *b, int len)
{
    float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
    for (int i = 0; i < len; i++) {
        dot += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    if (norm_a <= 0.0f || norm_b <= 0.0f) {
        return 0.0f;
    }
    return dot / (sqrtf(norm_a) * sqrtf(norm_b));
}

bool face_engine_extract_feat(const camera_fb_t *fb, float *out_feat, int feat_len)
{
    dl::image::img_t img = camera_fb_to_img(fb);
    std::list<dl::detect::result_t> &det_res = s_detect->run(img);

    const dl::detect::result_t *face = pick_largest_face(det_res);
    if (face == nullptr) {
        return false;
    }

    dl::TensorBase *tensor = s_feat->run(img, face->keypoint);
    return tensor_to_float(tensor, out_feat, feat_len);
}

face_identify_result_t face_engine_identify(const camera_fb_t *fb)
{
    face_identify_result_t result = {};

    dl::image::img_t img = camera_fb_to_img(fb);
    std::list<dl::detect::result_t> &det_res = s_detect->run(img);

    const dl::detect::result_t *face = pick_largest_face(det_res);
    if (face == nullptr) {
        return result; /* face_found = false */
    }
    result.face_found = true;

    float query_feat[OPERATOR_FEAT_LEN];
    dl::TensorBase *tensor = s_feat->run(img, face->keypoint);
    if (!tensor_to_float(tensor, query_feat, OPERATOR_FEAT_LEN)) {
        return result;
    }

    float threshold = settings_get_threshold();
    float best_similarity = 0.0f;
    int best_id = -1;

    int count = operator_store_count();
    for (int i = 0; i < count; i++) {
        const operator_record_t *rec = operator_store_get_by_index(i);
        float sim = cosine_similarity(query_feat, rec->feat, OPERATOR_FEAT_LEN);
        if (sim > best_similarity) {
            best_similarity = sim;
            best_id = rec->id;
        }
    }

    result.similarity = best_similarity;
    if (best_id >= 0 && best_similarity >= threshold) {
        result.matched = true;
        result.operator_id = (uint8_t)best_id;
    }
    return result;
}
