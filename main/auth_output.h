/*
 * auth_output.h
 * -----------------------------------------------------------------------
 * Controls the relay/optocoupler output and the status LEDs.
 *
 * ############################################################################
 * # SAFETY WARNING - READ THIS                                               #
 * #                                                                          #
 * # 1) The "permission" signal this module produces does NOT START a machine #
 * #    DIRECTLY. It only arms the machine's physical start button. The       #
 * #    operator must STILL press that physical start button to run the       #
 * #    machine. Never wire the relay output directly to motor/contactor      #
 * #    power or to a PLC's "run" line.                                       #
 * #                                                                          #
 * # 2) This system has NO LIVENESS DETECTION. It can be fooled by a          #
 * #    photograph or by an image shown on a screen. A real/production        #
 * #    authorization system MUST add a second factor (e.g. an RFID card read #
 * #    or a PIN entry, combined with AND logic).                             #
 * #                                                                          #
 * # 3) Electromechanical safety circuits such as the emergency-stop button   #
 * #    and door/guard switches must be designed as separate circuits that    #
 * #    cut power directly and are COMPLETELY INDEPENDENT of this software.   #
 * #    Even if the software hangs, produces a wrong "permission", or         #
 * #    crashes and resets, those physical safety measures must still be able #
 * #    to stop the machine. No software output can REPLACE those circuits.   #
 * ############################################################################
 */
#pragma once

#include "esp_err.h"

esp_err_t auth_output_init(void);

/* Drives the relay output HIGH, turns the green LED on and the red LED off,
 * and schedules an automatic return to LOW after `seconds` seconds
 * (via esp_timer - non-blocking, the calling task is not blocked). */
void auth_output_grant(uint32_t seconds);

/* Keeps the relay output LOW, briefly lights the red LED (denied indication),
 * and turns the green LED off. */
void auth_output_deny(void);

/* Initial / idle state: relay LOW, both LEDs off. */
void auth_output_idle(void);
