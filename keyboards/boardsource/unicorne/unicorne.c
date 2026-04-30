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
#include "eeprom_driver.h"
#include "timer.h"

bool buffer_written = false;
bool correct_data   = false;
const uint32_t OLED_EEPROM_OFFSET = 32;

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
        buffer_written = true;
    }
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(KEYBOARD_SYNC_A, keyboard_sync_a_slave_handler);

    // load oled buffer from EEPROM
    eeprom_read_block(slave_oled_buffer, (void*)OLED_EEPROM_OFFSET, sizeof(slave_oled_buffer));
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

        static uint32_t last_write = 0;

        if (buffer_written && timer_elapsed32(last_write) > 30000) { // 30 seconds
            eeprom_update_block(slave_oled_buffer, (void*)OLED_EEPROM_OFFSET, sizeof(slave_oled_buffer));
            last_write = timer_read32();
            buffer_written = false;
        }
    }
    return true;
}
#endif
