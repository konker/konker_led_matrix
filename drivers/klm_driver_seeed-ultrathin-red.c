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


#include "klm_matrix.h"
#include "klm_segment.h"


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
void klm_mat_mask_pixel(klm_matrix * const matrix, int16_t x, int16_t y, bool mask) {
    size_t p = KLM_BUF_OFFSET(matrix, x, y);
    bitWrite(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH,
            bitRead(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH) ^ mask);
}

/** Query whether or not the given pixel has been set */
bool klm_mat_is_pixel_set(klm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KLM_BUF_OFFSET(matrix, x, y);
#ifndef KLM_NO_DOUBLE_BUFFER
    if (bitRead(matrix->display_buffer1[p], x % KLM_BYTE_WIDTH) == KLM_ON) {
        return true;
    }
#else
    if (bitRead(matrix->display_buffer0[p], x % KLM_BYTE_WIDTH) == KLM_ON) {
        return true;
    }
#endif
    return false;
}

/** Clear the entire matrix */
void klm_mat_clear(klm_matrix *matrix) {
    int16_t i;
#ifndef KLM_NO_DOUBLE_BUFFER
    for (i=0; i<(matrix->height*matrix->_row_width); i++) {
        matrix->display_buffer1[i] = KLM_OFF_BYTE;
    }
#endif
    for (i=0; i<(matrix->height*matrix->_row_width); i++) {
        matrix->display_buffer0[i] = KLM_OFF_BYTE;
    }
}

void klm_mat_dump_buffer(klm_matrix * const matrix) {
    int16_t i;
    for (i=0; i<matrix->height*matrix->_row_width; i++) {
        KLM_LOG(matrix, "%02x ", matrix->display_buffer0[i]);
    }
    KLM_LOG(matrix, "\n");
#ifndef KLM_NO_DOUBLE_BUFFER
    for (i=0; i<matrix->height*matrix->_row_width; i++) {
        KLM_LOG(matrix, "%02x ", matrix->display_buffer1[i]);
    }
    KLM_LOG(matrix, "\n");
#endif

    int16_t x, y;
    for (y=0; y<matrix->height; y++) {
        for (x=0; x<matrix->width; x++) {
            if (klm_mat_is_pixel_set(matrix, x, y)) {
                KLM_LOG(matrix, "# ");
            }
            else {
                KLM_LOG(matrix, ". ");
            }
        }
        KLM_LOG(matrix, "\n");
    }
    KLM_LOG(matrix, "\n");
}

/** Drive the matrix display */
void klm_mat_scan(klm_matrix * const matrix) {
    if (!matrix->on) return;

#ifndef KLM_NON_GPIO_MACHINE
    // Process each 8-pixel byte in the row
    uint8_t offset = KLM_ROW_OFFSET(matrix, matrix->_scan_row);

    // Process the row in reverse order
    int16_t x8;
    for (x8=matrix->_row_width-1; x8>=0; x8--) {
#ifndef KLM_NO_DOUBLE_BUFFER
        uint8_t pixel8 = matrix->display_buffer1[offset + x8];
#else
        uint8_t pixel8 = matrix->display_buffer0[offset + x8];
#endif

        // Write each pixel in the byte, in reverse order
        shiftOut(matrix->r1, matrix->clk, MSBFIRST, pixel8);
    }

    // Disable display
    digitalWrite(matrix->oe, HIGH);

    // Display the rows in reverse order
    uint16_t display_row = (matrix->height - 1 - matrix->_scan_row);

    // Select row
    digitalWrite(matrix->a, (display_row & 0x01));
    digitalWrite(matrix->b, (display_row & 0x02));
    digitalWrite(matrix->c, (display_row & 0x04));
    digitalWrite(matrix->d, (display_row & 0x08));

    // Latch data
    digitalWrite(matrix->stb, LOW);
    digitalWrite(matrix->stb, HIGH);
    digitalWrite(matrix->stb, LOW);

    // Enable display
    digitalWrite(matrix->oe, LOW);

    // Next row, wrap around at the bottom
    matrix->_scan_row = (matrix->_scan_row + 1) % matrix->height;

    // Tick the matrix animation loop
    if (matrix->_scan_row == 0) {
        klm_mat_tick(matrix);
    }
#endif
}

