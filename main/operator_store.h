/*
 * operator_store.h
 * -----------------------------------------------------------------------
 * Stores enrolled operators (id, name, face feature vector, enrollment time)
 * as fixed-size records in a separate FATFS partition ("facedb") on the
 * internal flash.
 *
 * A separate FATFS partition was chosen over NVS because each feature vector
 * takes ~2KB (512 floats); NVS blobs are impractical for repeated writes at
 * that size. A simple fixed-record file also shows students the "database"
 * idea concretely.
 */
#pragma once

#include <cstdint>
#include "esp_err.h"

#define OPERATOR_MAX_COUNT   10
#define OPERATOR_NAME_LEN     32
/* Feature vector length of the HumanFaceFeat (MFN_S8_V1) model.
 * face_engine_init() reads this from the actual model and verifies it; on a
 * mismatch it logs an error (see face_engine.cpp). */
#define OPERATOR_FEAT_LEN    512

struct operator_record_t {
    uint8_t id;
    char name[OPERATOR_NAME_LEN];
    float feat[OPERATOR_FEAT_LEN];
    uint32_t enrolled_at_s; /* Based on esp_timer_get_time(), in seconds */
};

/* Mounts the "facedb" partition as FATFS (formatting it on first run) and
 * loads any enrolled operators into the RAM cache. */
esp_err_t operator_store_init(void);

int operator_store_count(void);

/* index: 0..operator_store_count()-1. Returns nullptr when out of range. */
const operator_record_t *operator_store_get_by_index(int index);

const operator_record_t *operator_store_find(uint8_t id);

/* Adds a new operator (returns ESP_ERR_NO_MEM when the store is full). On
 * success the assigned id is written to out_id. */
esp_err_t operator_store_add(const char *name, const float *feat, int feat_len, uint8_t *out_id);

esp_err_t operator_store_delete(uint8_t id);
