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

/** Drive the matrix display */
void klm_mat_scan(klm_matrix * const matrix) {
    if (!matrix->on) return;
}

