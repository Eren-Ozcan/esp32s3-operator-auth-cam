# TODO — gaps and things to do

This project has never been built or flashed (there is no ESP-IDF toolchain in
this environment). The code was written against real API signatures verified
from GitHub, but it has never been tested on real hardware. The list below is
ordered by priority: what needs doing before it can be used safely in class.

## 1) Build / first flash (must be done)

- [ ] Run `idf.py set-target esp32s3` + `idf.py build` and fix the first round
      of compile errors. The three most likely spots are already marked in the
      code/README:
  - [ ] `face_engine.cpp` -> `tensor_to_float()`: verify against the real
        esp-dl version whether the model output is `float` or `int8`.
  - [ ] `operator_store.h` -> `OPERATOR_FEAT_LEN` (512 was assumed);
        `face_engine_init()` compares this at runtime and logs an error —
        update it if it does not match.
  - [ ] Verify that the version ranges for `espressif/esp32-camera`,
        `espressif/human_face_detect` and `espressif/human_face_recognition`
        in `idf_component.yml` are still valid in the component registry (new
        major versions can break the API over time).
- [ ] Flash the board and verify on the serial monitor that startup logs pass
      without errors (camera init, model loading, NVS, facedb mount).

## 2) Hardware bring-up (once you have the board)

- [ ] Compare the camera pins in `main/pin_config.h` against the board's real
      schematic — those values were assumptions, never verified.
- [ ] If the image comes out upside down or corrupted, try a value such as
      24MHz instead of the current `CAM_XCLK_FREQ_HZ` (20MHz).
- [ ] Test the relay/optocoupler module with only an LED/multimeter **before
      connecting it to the PLC** (verify that the GPIO21 output switches at the
      right level).
- [ ] Check that the relay module is powered from a separate supply and not
      from the board's 3.3V rail (the README already warns about this).
- [ ] Decide whether `ENROLL_BUTTON_GPIO` (BOOT/GPIO0) is actually usable while
      the board is running, or whether a separate physical button is needed.
- [ ] If you will use the SD card feature, enable `APP_ENABLE_SD_CARD` and
      verify that the SPI pins (39/40/41/42) are free/suitable on the board.

## 3) Calibration (in the classroom)

- [ ] The default threshold (`0.55`) should be tested under the classroom's
      lighting and tuned with `set-threshold` — find the false-reject /
      false-accept rate by trying it with several students.
- [ ] The default permission duration (5 s) should be tuned with
      `set-duration` to suit the demo scenario.
- [ ] Run enroll/recognize attempts with at least 3-5 different people and
      check for any visible false positives/negatives.

## 4) Known gaps / features not implemented

- [ ] **No bulk delete**: there is only `delete <id>`, no `clear-all` that
      wipes every record in one command (useful for resetting without deleting
      facedb by hand).
- [ ] **No enroll cancel**: once `enroll` has started, there is no
      command/button that cancels the 3-second countdown.
- [ ] **SD log file growth is not bounded**: `/sdcard/access_log.csv` is never
      rotated and has no size limit; on a demo board left running for a long
      time the file can grow.
- [ ] **Camera/model init failure -> boot loop**: because `app_main.cpp` uses
      `ESP_ERROR_CHECK`, the device resets continuously if the camera or the
      models cannot load; in a classroom that can cause "why does it keep
      resetting" confusion. A gentler error screen/log loop could be added.
- [ ] **Button debounce is crude**: `enroll_button_pressed_edge()` relies only
      on the main loop period (~150ms); hardware debounce or tighter software
      debounce could be added.

## 5) Improvement ideas (optional, not required)

- [ ] A "test mode" alongside the relay output (simulating with the LEDs, to
      demo the software without wiring a relay at all).
- [ ] Add a summary of the last 5 recognition attempts to the `status` command.
- [ ] A simple wiring diagram image (Fritzing/KiCad) to show students — right
      now there is only the text description.
- [ ] Add a BOM (bill of materials: relay model, LEDs, resistor values).
