/**
 * Konker's ultrathin LED matrix library
 *
 * A library for driving the Seeedstudio Ultrathin red LED matrix
 *
 * Copyright 2015, Konrad Markus <konker@luxvelocitas.com>
 *
 * This file is part of konker_ultrathin_led_matrix.
 *
 * konker_ultrathin_led_matrix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "kulm_matrix.h"
#include "kulm_segment.h"

// Macros for convenience
#define KULM_ROW_OFFSET(matrix, y) (matrix->_row_width*y)
#define KULM_BUF_OFFSET(matrix, x, y) (KULM_ROW_OFFSET(matrix, y)+x/8)
#define KULM_BUF_INDEX(x, y, w) (y*w + x)/KULM_BYTE_WIDTH
#define KULM_GET_PIXEL8(buf, x, y, w) buf[KULM_BUF_INDEX(x, y, w)];

// Symbolic constants
#define KULM_BYTE_WIDTH 8
#define KULM_CHARACTER_HEIGHT 6
#define KULM_CHARACTER_SPACING 1


/**
 * Create a new matrix
 *
 * @param display_buffer  A uint8_t array of length width * height to hold
 *                        the contents of the matrix display
 * @param width  The total width of the matrix, or matrices, in "pixels"
 * @param height  The total height of the matrix, or matrices, in "pixels"
 *
 * @return  A pointer to a newly initialized matrix stucture
 */
kulm_matrix * const kulm_mat_create(
                            uint8_t *display_buffer,
                            uint8_t width,
                            uint8_t height,
                            uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                            uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk)
{
    // Allocate memory for a kulm_matrix structure and initialize all members
    kulm_matrix * const matrix = malloc(sizeof(kulm_matrix));

    matrix->width = width;
    matrix->height = height;
    matrix->display_buffer = display_buffer;
    matrix->_row_width = (width / KULM_BYTE_WIDTH);

    matrix->a = a;
    matrix->b = b;
    matrix->c = c;
    matrix->d = d;
    matrix->oe = oe;
    matrix->r1 = r1;
    matrix->stb = stb;
    matrix->clk = clk;

    matrix->on = true;
    matrix->paused = false;
    matrix->_scan_row = 0;

    return matrix;
}

/** Clean up a matrix object */
void kulm_mat_destroy(kulm_matrix * const matrix) {
    // Free dynamically allocated memory
    free(matrix);
}

/** Initialize a matrix object with a set of fonts and a set of segments */
void kulm_mat_init(kulm_matrix * const matrix,
                   hexfont_list *font_list,
                   kulm_segment ** const segments,
                   uint16_t num_segments)
{
    matrix->font_list = font_list;
    matrix->segments = segments;
    matrix->num_segments = num_segments;

    // Initilize pin modes
    pinMode(matrix->a, OUTPUT);
    pinMode(matrix->b, OUTPUT);
    pinMode(matrix->c, OUTPUT);
    pinMode(matrix->d, OUTPUT);
    pinMode(matrix->oe, OUTPUT);
    pinMode(matrix->r1, OUTPUT);
    pinMode(matrix->clk, OUTPUT);
    pinMode(matrix->stb, OUTPUT);
}

/** Drive the matrix display */
void kulm_mat_scan(kulm_matrix *matrix) {
    if (!matrix->on) return;

    // Process each 8-pixel byte in the row
    uint8_t offset = KULM_ROW_OFFSET(matrix, matrix->_scan_row);

    // Process the row in reverse order
    int16_t x8;
    for (x8=matrix->_row_width-1; x8>=0; x8--) {
        uint8_t pixel8 = matrix->display_buffer[offset + x8];

        /*[XXX: remove]
        // Apply the mask
        pixel8 ^= matrix->mask;
        */

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
}

/** Drive animation */
void kulm_mat_tick(kulm_matrix *matrix) {
    if (matrix->paused) return;

    int16_t i;
    for (i=0; i<matrix->num_segments; i++) {
        kulm_seg_tick(matrix->segments[i]);
    }
}

/** Switch a matrix pixel on */
void kulm_mat_set_pixel(kulm_matrix *matrix, int16_t x, int16_t y) {
    bitWrite(matrix->display_buffer[KULM_BUF_OFFSET(matrix, x, y)], x % KULM_BYTE_WIDTH, 1);
}

/** Switch a matrix pixel off */
void kulm_mat_clear_pixel(kulm_matrix *matrix, int16_t x, int16_t y) {
    bitWrite(matrix->display_buffer[KULM_BUF_OFFSET(matrix, x, y)], x % KULM_BYTE_WIDTH, 0);
}

/** Clear the entire matrix */
void kulm_mat_clear(kulm_matrix *matrix) {
    int16_t i;
    for (i=0; i<(matrix->height*matrix->_row_width); i++) {
        matrix->display_buffer[i] = 0x00;
    }
}

/** Clear a region of the matrix */
void kulm_mat_clear_region(kulm_matrix *matrix, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    int16_t by, bx;

    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t X = x+bx;
            kulm_mat_clear_pixel(matrix, X, y+by);
        }
    }
}

/** Start animation of matrix content */
void kulm_mat_start(kulm_matrix *matrix) {
    matrix->paused = false;
}

/** Stop animation of matrix content */
void kulm_mat_stop(kulm_matrix *matrix) {
    matrix->paused = true;
}

/** Switch off matrix display altogether */
void kulm_mat_on(kulm_matrix *matrix) {
    matrix->on = true;
}

/** Switch on matrix display */
void kulm_mat_off(kulm_matrix *matrix) {
    matrix->on = false;
    digitalWrite(matrix->oe, HIGH);
}

/** Reverse the matrix display */
void kulm_mat_reverse(kulm_matrix *matrix) {
    int16_t i;
    for (i=0; i<matrix->num_segments; i++) {
        kulm_seg_reverse(matrix->segments[i]);
    }
}

/** Set a region of pixels from a source sprite array */
void kulm_mat_render_sprite(kulm_matrix * const matrix, hexfont_character * const sprite, int16_t x, int16_t y) {
    int16_t by, bx;

    for (by=0; by<sprite->height; by++) {
        for (bx=0; bx<sprite->height; bx++) {
            int16_t _x = x+bx;
            int16_t _y = y+by;

            if (_x >= matrix->width || _x < 0) {
                continue;
            }

            if (hexfont_get_pixel(sprite, bx, by)) {
                kulm_mat_set_pixel(matrix, _x, _y);
            }
            else {
                kulm_mat_clear_pixel(matrix, _x, _y);
            }
        }
    }
}

