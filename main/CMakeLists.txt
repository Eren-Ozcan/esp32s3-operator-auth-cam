idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES
        esp32-camera
        human_face_detect
        human_face_recognition
        console
        nvs_flash
        fatfs
        wear_levelling
        sdmmc
        esp_timer
        driver
)

# HumanFaceFeat::run() ve dl::TensorBase, dl::image ic API'lerinde C++
# exception kullanilmadigi icin ozel bir derleyici bayragina gerek yok;
# yalnizca C++17 yeterli (idf_component_register varsayilan standardi).
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
