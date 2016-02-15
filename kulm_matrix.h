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

#ifndef __KONKER_ULTRATHIN_LED_MATRIX_H__
#define __KONKER_ULTRATHIN_LED_MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#  include <Arduino.h>
#else
#  ifndef NON_GPIO_MACHINE
#    include <wiringPi.h>
#    include <wiringShift.h>
#  endif
// Why aren't these in wiringPi?
#  define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#  define bitSet(value, bit) ((value) |= (1UL << (bit)))
#  define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#  define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

#include <hexfont.h>
#include <hexfont_list.h>
#include "kulm_segment.h"
#include "kulm_segment_list.h"

// Symbolic constants
#define KULM_BYTE_WIDTH 8
#define KULM_OFF_BYTE 0xff
#define KULM_OFF 0x01
#define KULM_ON 0x00

// Macros for convenience
#define KULM_BUFFER_LEN(w, h) (h * (w/KULM_BYTE_WIDTH))
#define KULM_ROW_OFFSET(matrix, y) (matrix->_row_width*y)
#define KULM_BUF_OFFSET(matrix, x, y) (KULM_ROW_OFFSET(matrix, y)+x/KULM_BYTE_WIDTH)


// Forward declare kulm_segment because of circular refs
typedef struct kulm_segment kulm_segment;

typedef struct kulm_matrix
{
    // GPIO control pins
    uint8_t a, b, c, d, oe, r1, stb, clk;

    // Physical dimensions of the display
    int16_t width;
    int16_t height;

    // A buffer to hold the current frame
    uint8_t *display_buffer;

    // A list of available fonts and associated font-metrics
    hexfont_list *font_list;

    // Global matrix state flags
    bool on;
    bool paused;

    // A list of virtual segments which make up the display
    kulm_segment_list *segment_list;

    // Internal vars
    uint16_t _row_width;
    uint16_t _scan_row;

} kulm_matrix;


/** Create a matrix object by specifying its physical characteristics */
kulm_matrix * const kulm_mat_create(
                            uint8_t *display_buffer,
                            uint8_t width,
                            uint8_t height,
                            uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                            uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk);

/** Clean up a matrix object */
void kulm_mat_destroy(kulm_matrix * const matrix);

/** Initialize a matrix object with a set of fonts and a set of segments */
void kulm_mat_init(
            kulm_matrix * const matrix,
            hexfont_list * const font_list,
            kulm_segment_list * const segment_list);

/** Initialize a matrix object with the given font. A full-screen segment will be automatically created */
void kulm_mat_simple_init(kulm_matrix * const matrix, hexfont * const font);

/** Set the default full-screen segment's text content */
uint16_t kulm_mat_simple_set_text(kulm_matrix * const matrix, const char *text);

/** Set the animation scroll speed of the default full-screen segment in pixels per frame */
void kulm_mat_simple_set_text_speed(kulm_matrix * const matrix, float speed);

/** Drive animation */
void kulm_mat_tick(kulm_matrix * const matrix);

/** Query whether or not the given pixel has been set */
bool kulm_mat_is_pixel_set(kulm_matrix * const matrix, int16_t x, int16_t y);

/** Clear the entire matrix */
void kulm_mat_clear(kulm_matrix * const matrix);

/** Start animation of matrix content */
void kulm_mat_start(kulm_matrix * const matrix);

/** Stop animation of matrix content */
void kulm_mat_stop(kulm_matrix * const matrix);

/** Switch off matrix display altogether */
void kulm_mat_on(kulm_matrix * const matrix);

/** Switch on matrix display */
void kulm_mat_off(kulm_matrix * const matrix);

/** Print a representation of the display buffer to the console */
void kulm_mat_dump_buffer(kulm_matrix * const matrix, FILE *fp);

// Inline funtions
// ----------------------------------------------------------------------------

/** Switch a matrix pixel on */
inline void kulm_mat_set_pixel(kulm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KULM_BUF_OFFSET(matrix, x, y);
    bitWrite(matrix->display_buffer[p], x % KULM_BYTE_WIDTH, KULM_ON);
}

/** Switch a matrix pixel off */
inline void kulm_mat_clear_pixel(kulm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KULM_BUF_OFFSET(matrix, x, y);
    bitWrite(matrix->display_buffer[p], x % KULM_BYTE_WIDTH, KULM_OFF);
}

/** Clear a region of the matrix */
inline void kulm_mat_clear_region(
                    kulm_matrix * const matrix,
                    int16_t x, int16_t y,
                    uint16_t w, uint16_t h)
{
    int16_t bx, by;
    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t _x = x + bx;
            int16_t _y = y + by;

            kulm_mat_clear_pixel(matrix, _x, _y);
        }
    }
}

/** Set a region of pixels from a source sprite array */
inline void kulm_mat_render_sprite(
                    kulm_matrix * const matrix,
                    hexfont_character * const sprite,
                    int16_t x, int16_t y,
                    int16_t clip_x0, int16_t clip_y0,
                    int16_t clip_x1, int16_t clip_y1)
{
    int16_t by, bx;
    for (by=0; by<sprite->height; by++) {
        for (bx=0; bx<sprite->width; bx++) {
            int16_t _x = x + bx;
            int16_t _y = y + by;

            if (_x < clip_x0 || _x >= clip_x1 ||
                _y < clip_y0 || _y >= clip_y1)
            {
                continue;
            }

            if (hexfont_character_get_pixel(sprite, bx, by)) {
                kulm_mat_set_pixel(matrix, _x, _y);
            }
            else {
                kulm_mat_clear_pixel(matrix, _x, _y);
            }
        }
    }
}

/** Drive the matrix display */
inline void kulm_mat_scan(kulm_matrix * const matrix) {
    if (!matrix->on) return;

#ifndef NON_GPIO_MACHINE
    // Process each 8-pixel byte in the row
    uint8_t offset = KULM_ROW_OFFSET(matrix, matrix->_scan_row);

    // Process the row in reverse order
    int16_t x8;
    for (x8=matrix->_row_width-1; x8>=0; x8--) {
        uint8_t pixel8 = matrix->display_buffer[offset + x8];

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

#ifdef __cplusplus
}
#endif

#endif // __KONKER_ULTRATHIN_LED_MATRIX_H__
