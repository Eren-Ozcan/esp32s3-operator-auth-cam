/*
 * serial_cli.h
 * -----------------------------------------------------------------------
 * esp_console-based command line: enroll <name>, delete <id>, list,
 * set-threshold <value>, set-duration <seconds>, status.
 *
 * Because the camera and the face recognition engine run in a single main
 * loop task (app_main.cpp), the "enroll" command does NOT perform the
 * enrollment ITSELF; it only leaves a request behind. The main loop picks the
 * request up on the next frame and carries out the enrollment (this keeps
 * camera access in one place and avoids race conditions).
 */
#pragma once

#include "esp_err.h"
#include <cstddef>

esp_err_t serial_cli_init(void);

/* Called from the console command or the button interrupt handler. If a
 * request is already pending, the new one is ignored (and logged). */
void serial_cli_request_enroll(const char *name);

/* Called by the main loop on every iteration. If a request is pending, copies
 * it into name_out, returns true, and consumes the request. */
bool serial_cli_take_enroll_request(char *name_out, size_t max_len);
