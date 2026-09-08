/*
 * pin_config.h
 * -----------------------------------------------------------------------
 * ALL GPIO PIN DEFINITIONS ARE COLLECTED IN THIS FILE.
 *
 * Boards sold under the label "ESP32-S3 AI CAM" have NO single official or
 * standard pin layout (it varies between clones). The camera pins below are
 * chosen as an ASSUMPTION based on the layout most commonly seen on this type
 * of board (Freenove / generic ESP32-S3-CAM style).
 *
 * >>> If your board's silkscreen/schematic differs, update ONLY this file. <<<
 *
 * -----------------------------------------------------------------------
 * SAFETY WARNING (summary - details in README.md and auth_output.h):
 *   - The "permission" signal this system produces does NOT start a machine
 *     directly; it only arms the physical start button. The operator must
 *     still press that physical button.
 *   - This system has NO LIVENESS DETECTION. It can be fooled by a
 *     photograph. A real/production authorization system MUST add a second
 *     factor (RFID card, PIN code, etc.).
 *   - Electromechanical safety circuits such as the emergency stop and door
 *     switches must be COMPLETELY INDEPENDENT of this software. Even if the
 *     software hangs, grants a wrong "permission", or crashes, those physical
 *     safety circuits must still be able to stop the machine.
 * -----------------------------------------------------------------------
 */
#pragma once

#include "driver/gpio.h"

/* ======================= CAMERA PINS (OV3660, onboard) ======================= */
/* The camera module comes soldered to the board, so these pins need no
 * external wiring; only the correct constants have to be selected. */
#define CAM_PIN_PWDN    (-1)   /* Not connected on most clones */
#define CAM_PIN_RESET   (-1)   /* Not connected on most clones */
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4      /* SCCB/I2C SDA */
#define CAM_PIN_SIOC    5      /* SCCB/I2C SCL */

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11

#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

#define CAM_XCLK_FREQ_HZ  20000000

/* ======================= AUTHORIZATION OUTPUT ======================= */
/* Connects to the IN pin of the optocoupler/relay module. The relay output
 * (NO/COM) goes to the PLC input or the contactor coil - see the wiring
 * section in the README. */
#define RELAY_OUTPUT_GPIO   GPIO_NUM_21

/* Status LEDs (GPIO -> 220-330 ohm resistor -> LED anode, cathode -> GND) */
#define LED_GREEN_GPIO       GPIO_NUM_47   /* Granted */
#define LED_RED_GPIO         GPIO_NUM_48   /* Denied / idle */

/* ======================= ENROLLMENT BUTTON ======================= */
/* Most AI-CAM clone boards have no user button other than BOOT. BOOT (GPIO0)
 * can be used as a normal input pin AFTER startup (just do not confuse it with
 * its role during boot/reset: a short press while the board is running is
 * fine). If your board has a separate button, change this value. */
#define ENROLL_BUTTON_GPIO   GPIO_NUM_0
#define ENROLL_BUTTON_ACTIVE_LOW  1

/* ======================= SD CARD (OPTIONAL, SPI MODE) ======================= */
/* Used only when CONFIG_APP_ENABLE_SD_CARD is enabled (see Kconfig.projbuild). */
#define SD_SPI_CS_GPIO     GPIO_NUM_39
#define SD_SPI_MOSI_GPIO   GPIO_NUM_40
#define SD_SPI_MISO_GPIO   GPIO_NUM_41
#define SD_SPI_SCK_GPIO    GPIO_NUM_42
