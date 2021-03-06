/**
 * Konker's LED matrix library
 *
 * A library for driving a red LED matrix
 *
 * Copyright 2015, Konrad Markus <konker@luxvelocitas.com>
 *
 * This file is part of konker_led_matrix.
 *
 * konker_led_matrix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * konker_led_matrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with konker_led_matrix.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <unistd.h>
#include "klm_matrix.h"
#include "klm_segment.h"

#define SCAN_LOOP_DELAY_MICROS 400


/** Switch a matrix pixel on */
void klm_mat_set_pixel(klm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KLM_BUF_OFFSET(matrix, x, y);
    bitWrite(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH, KLM_ON);
}

/** Switch a matrix pixel off */
void klm_mat_clear_pixel(klm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KLM_BUF_OFFSET(matrix, x, y);
    bitWrite(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH, KLM_OFF);
}

/** Switch a matrix pixel off */
void klm_mat_mask_pixel(klm_matrix * const matrix, int16_t x, int16_t y, bool reverse) {
    size_t p = KLM_BUF_OFFSET(matrix, x, y);
    bitWrite(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH,
            bitRead(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH) ^ reverse);
}

/** Query whether or not the given pixel has been set */
bool klm_mat_is_pixel_set(klm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KLM_BUF_OFFSET(matrix, x, y);
    if (bitRead(matrix->display_buffer1[p], x % KLM_BYTE_WIDTH) == KLM_ON) {
        return true;
    }
    return false;
}

/** Drive the matrix display */
void klm_mat_scan(klm_matrix * const matrix) {
    if (!matrix->on) return;

#ifndef KLM_NON_GPIO_MACHINE
    matrix->scan_row = 0;
    uint16_t loop_limit = matrix->config->height;
    uint16_t mod_limit = loop_limit + 1;

    while (matrix->scan_row < loop_limit) {
        // Process each 8-pixel byte in the row
        uint8_t offset = KLM_ROW_OFFSET(matrix, matrix->scan_row);

        // Process the row in reverse order
        int16_t x8;
        for (x8=matrix->_row_width-1; x8>=0; x8--) {
            uint8_t pixel8 = matrix->display_buffer1[offset + x8];

            // Flip all bits
            pixel8 ^= 0xFF;

            // Write each pixel in the byte, in reverse order
            shiftOut(klm_config_get_pin(matrix->config, 'r'),
                     klm_config_get_pin(matrix->config, 'x'),
                     MSBFIRST,
                     pixel8);
        }

        // Disable display
        digitalWrite(klm_config_get_pin(matrix->config, 'o'), HIGH);

        // Display the rows in reverse order
        uint16_t display_row = (matrix->config->height - 1 - matrix->scan_row);

        // Select row
        digitalWrite(klm_config_get_pin(matrix->config, 'a'), (display_row & 0x01));
        digitalWrite(klm_config_get_pin(matrix->config, 'b'), (display_row & 0x02));
        digitalWrite(klm_config_get_pin(matrix->config, 'c'), (display_row & 0x04));
        digitalWrite(klm_config_get_pin(matrix->config, 'd'), (display_row & 0x08));

        // Latch data
        digitalWrite(klm_config_get_pin(matrix->config, 's'), LOW);
        digitalWrite(klm_config_get_pin(matrix->config, 's'), HIGH);
        digitalWrite(klm_config_get_pin(matrix->config, 's'), LOW);

        // Enable display
        digitalWrite(klm_config_get_pin(matrix->config, 'o'), LOW);

        // Next row, wrap around at the bottom
        matrix->scan_row = (matrix->scan_row + 1) % mod_limit;

        // Avoid a tight loop using all the CPU
        usleep(SCAN_LOOP_DELAY_MICROS);
    }
#endif
}

void klm_mat_init_hardware(klm_matrix * const matrix) {
#ifndef KLM_NON_GPIO_MACHINE
    // Initilize pin modes
    pinMode(klm_config_get_pin(matrix->config, 'a'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 'b'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 'c'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 'd'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 'o'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 'r'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 'x'), OUTPUT);
    pinMode(klm_config_get_pin(matrix->config, 's'), OUTPUT);
#endif
}

void klm_mat_init_display_buffer(klm_matrix * const matrix) {
    matrix->display_buffer0 =
        calloc(KLM_BUFFER_LEN(matrix->config->height, matrix->config->width),
               sizeof(*matrix->display_buffer0));

    matrix->display_buffer1 =
        calloc(KLM_BUFFER_LEN(matrix->config->height, matrix->config->width),
               sizeof(*matrix->display_buffer1));

    klm_mat_clear(matrix);
}

