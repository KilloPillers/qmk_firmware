// Copyright 2024 jack (@waffle87)
// SPDX-License-Identifier: GPL-2.0-or-later
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "oled_driver.h"
#include "print.h"
#include "quantum.h"
#include "lib/oled.h"
#include "transactions.h"
#include "transport.h"
#include "unicorne.h"

bool buffer_written = false;
bool correct_data   = false;

void keyboard_sync_a_slave_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    static uint16_t offset = 0;
    // ensure no overflow
    if (offset + in_buflen > sizeof(slave_oled_buffer)) {
        offset = 0;
    }

    memcpy(slave_oled_buffer + offset, in_data, in_buflen);

    offset += in_buflen;
    if (offset >= sizeof(slave_oled_buffer)) {
        offset = 0;
    }
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(KEYBOARD_SYNC_A, keyboard_sync_a_slave_handler);
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    if (!is_keyboard_master()) {
        return OLED_ROTATION_180;
    }
    return rotation;
}

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }
    if (is_keyboard_master()) {
        render_layer_state();
    } else {
        oled_write_raw((char *)slave_oled_buffer, sizeof(slave_oled_buffer));
    }
    return true;
}
#endif
