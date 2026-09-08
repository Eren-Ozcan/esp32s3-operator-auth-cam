/*
 * face_engine.h
 * -----------------------------------------------------------------------
 * Uses ESP-DL's HumanFaceDetect (face detection) and HumanFaceFeat (feature
 * extraction) classes directly.
 *
 * KEY DESIGN NOTE: esp-who's ready-made HumanFaceRecognizer class
 * (esp-dl/models/human_face_recognition) hardcodes the similarity threshold
 * (m_thr) to 0.5 in its constructor and offers NO public method to change it.
 * This project required the threshold to be configurable, so
 * HumanFaceRecognizer is skipped; instead the feature vector is extracted with
 * HumanFaceFeat, and the comparison (cosine similarity) and threshold check
 * are done here, in our own code. This also lets students see the "similarity
 * threshold" concept concretely in the code.
 */
#pragma once

#include "esp_err.h"
#include "esp_camera.h"
#include "operator_store.h"

struct face_identify_result_t {
    bool face_found;      /* Whether at least one face was detected in the frame */
    bool matched;         /* Whether a match above the threshold was found */
    uint8_t operator_id;   /* Valid when matched == true */
    float similarity;      /* Similarity score of the best candidate (0..1) */
};

esp_err_t face_engine_init(void);

/* Compares the largest (closest to the camera) face in the frame against the
 * enrolled operators. Returns face_found=false when there is no face. */
face_identify_result_t face_engine_identify(const camera_fb_t *fb);

/* Extracts the feature vector of the largest face in the frame (for
 * enrollment). out_feat must be allocated for at least OPERATOR_FEAT_LEN
 * elements. Returns false when no face is found. */
bool face_engine_extract_feat(const camera_fb_t *fb, float *out_feat, int feat_len);
