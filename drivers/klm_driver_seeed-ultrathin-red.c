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
#endif
}

