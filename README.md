# ESP32-S3 AI CAM — Operator-Recognition Machine Authorization System

An educational demo for a vocational high school electrical/electronics
workshop. When face recognition verifies an enrolled operator, it produces a
"permission to run" signal on a GPIO (relay/optocoupler -> PLC input or
contactor coil).

## ⚠️ SAFETY WARNING (read this first)

1. **The "permission" signal this system produces does NOT start the machine
   directly.** It only arms the machine's physical start button. The operator
   must still press that physical start button to run the machine. Never wire
   the relay output directly to motor/contactor power or to a PLC's "run" line.
2. **This system has NO liveness detection.** It can be fooled by a photograph
   or by an image shown on a screen. A real/production authorization system
   must add a second factor (e.g. an RFID card read or a PIN entry, combined
   with AND logic).
3. **Safety circuits such as the emergency-stop button and door/guard switches
   must be completely independent of this software.** Those physical safety
   measures must still be able to stop the machine even if the software hangs,
   produces a wrong "permission", or crashes and resets.

## Hardware

- Board: ESP32-S3 AI CAM (OV3660 3MP camera, 8MB PSRAM, 16MB Flash)
- 1x optocoupler/relay module (GPIO -> PLC input / contactor coil)
- 2x LED (green: granted, red: denied) + 220-330Ω resistors
- Optional: microSD card module (SPI)

### Pin map (`main/pin_config.h`)

Boards sold under the name "ESP32-S3 AI CAM" have no single official pinout;
the camera pins below are an **assumption** based on common clones. If your
board differs, **update only `main/pin_config.h`** — no other file needs to be
touched.

| Signal | GPIO |
|---|---|
| XCLK | 15 |
| SIOD/SDA | 4 |
| SIOC/SCL | 5 |
| D0..D7 | 11, 9, 8, 10, 12, 18, 17, 16 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |
| Relay output | 21 |
| Green LED | 47 |
| Red LED | 48 |
| Enroll button | 0 (BOOT) |
| SD CS / MOSI / MISO / SCK (optional) | 39 / 40 / 41 / 42 |

### Wiring (in words)

- **Camera**: soldered/onboard, no external wiring needed.
- **Relay module**: `RELAY_OUTPUT_GPIO` (GPIO21) -> the relay module's IN pin.
  Where possible, power the relay module's VCC/GND from a separate 3.3V/5V
  supply (the coil current can strain the board regulator). The relay NO/COM
  contacts go to the PLC input or the contactor coil.
- **Green LED**: GPIO47 -> 220-330Ω resistor -> LED anode; cathode -> GND.
- **Red LED**: GPIO48 -> same arrangement.
- **SD card (optional)**: SPI mode, CS=39, MOSI=40, MISO=41, SCK=42.

## Software architecture

```
main/
  pin_config.h        - All pin definitions (single place)
  app_main.cpp         - Startup sequence + main loop
  camera_module.*      - Camera init + img_t conversion
  face_engine.*         - HumanFaceDetect + HumanFaceFeat, with our own
                          cosine-similarity matching and configurable threshold
  operator_store.*      - Operator records (FATFS "facedb" partition)
  settings.*            - NVS: threshold and permission duration
  auth_output.*         - Relay + LED control, auto-off after N seconds
  access_log.*          - Serial port + optional SD CSV log
  sd_card.*             - Optional SPI SD mount
  serial_cli.*          - Command line interface
```

### Key design note: why custom threshold logic?

esp-who's ready-made `HumanFaceRecognizer` class hardcodes the similarity
threshold (`0.5`) and offers no public method to change it. This project wanted
the threshold to be configurable, so it uses `HumanFaceDetect` +
`HumanFaceFeat` directly; the comparison (cosine similarity) and the threshold
check are done by our own code in `face_engine.cpp`. A side benefit is that
students can see the "similarity threshold" concept directly in the code.

## Enrolling an operator

Two ways:

1. **Via serial command (preferred, since you can supply a name):**
   ```
   operator-auth> enroll Ahmet
   ```
   The system shows a 3-second countdown, the operator looks at the camera, the
   face is detected, its feature vector is extracted and stored.

2. **Via the BOOT button (automatic name, since you cannot type one):** with the
   board running, a short press of BOOT starts the same flow with an automatic
   name like `operator_N`. You can match ids to people afterwards with `list`.

### Other commands

| Command | Description |
|---|---|
| `list` | Lists enrolled operators with id/name/enrollment time |
| `delete <id>` | Deletes an operator |
| `set-threshold <0..1>` | Sets the similarity threshold (default `0.55`) |
| `set-duration <s>` | Sets the permission duration (default `5`) |
| `status` | Shows the current threshold, duration and operator count |

At most **10 operators** are supported. `enroll` is rejected when full.

## Setup

### 1. Installing ESP-IDF v5.2 (Windows)

- Install ESP-IDF v5.2 by following the
  [ESP-IDF Windows setup guide](https://docs.espressif.com/projects/esp-idf/en/v5.2/esp32s3/get-started/windows-setup.html)
  (either with the VS Code "ESP-IDF" extension or the "ESP-IDF Tools"
  installer).
- In VS Code, select the v5.2 environment via
  `ESP-IDF: Configure ESP-IDF Extension`.

### 2. Opening the project and setting the target

```
idf.py set-target esp32s3
```

This step downloads the dependencies listed in `idf_component.yml`
(esp32-camera, human_face_detect, human_face_recognition — these pull in
esp-dl automatically) from the internet.

### 3. Checking menuconfig

`sdkconfig.defaults` already contains the required settings (PSRAM Octal/80MHz,
custom partition table, USB-Serial/JTAG console, C++ exceptions, model storage
location). If you want to verify them anyway:

```
idf.py menuconfig
```

- `Component config -> ESP PSRAM` -> Octal Mode, 80MHz must be enabled.
- `Serial flasher config -> Flash size` -> 16 MB.
- `Component config -> human_face_detect` / `human_face_recognition` -> the
  model variant (ESPDET_PICO_224_224_FACE / MFN_S8_V1) must be selected.
- If you will use an SD card: enable
  `Operator Authorization Application -> Write access log to SD card`.

### 4. Build, flash, monitor

```
idf.py build
idf.py -p COMx flash monitor
```

(Replace `COMx` with the serial port the board is attached to; you can check it
in Device Manager.)

### Things you may hit while building

- **`HumanFaceFeat`/`TensorBase` data type**: the `tensor_to_float()` function
  in `face_engine.cpp` takes one of two paths depending on whether the model
  output is `float` or `int8` (quantized). If your esp-dl version returns a
  different type, the compiler or runtime will warn; update that function to
  match that version's `dl_tensor_base.hpp` definition.
- **Feature vector length**: `OPERATOR_FEAT_LEN` (512) in `operator_store.h` is
  compared against `HumanFaceFeat::get_feat_len()` (inside
  `face_engine_init()`); on a mismatch it logs a clear error and halts startup.
- **Camera pin/format compatibility**: OV3660 on a different clone may need a
  different XCLK frequency; if the image is corrupted, try a value such as
  24000000 for `CAM_XCLK_FREQ_HZ` (in `pin_config.h`).
