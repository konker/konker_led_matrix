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

#ifndef __KONKER_LED_MATRIX_H__
#define __KONKER_LED_MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef ARDUINO
#  include <Arduino.h>
#else
#  ifndef KLM_NON_GPIO_MACHINE
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
#include "klm_segment.h"
#include "klm_segment_list.h"

// Symbolic constants
#define KLM_BYTE_WIDTH 8
#define KLM_OFF_BYTE 0x00
#define KLM_OFF 0x00
#define KLM_ON 0x01

// Macros for convenience
#define KLM_BUFFER_LEN(w, h) (h * (w/KLM_BYTE_WIDTH))
#define KLM_ROW_OFFSET(matrix, y) (matrix->_row_width*y)
#define KLM_BUF_OFFSET(matrix, x, y) (KLM_ROW_OFFSET(matrix, y)+x/KLM_BYTE_WIDTH)
#define KLM_LOG(matrix, ...) fprintf(matrix->logfp, __VA_ARGS__); \
                              fflush(matrix->logfp);

// Forward declare driver methods
extern void klm_mat_scan(klm_matrix * const matrix);
extern void klm_mat_set_pixel(klm_matrix * const matrix, int16_t x, int16_t y);
extern void klm_mat_clear_pixel(klm_matrix * const matrix, int16_t x, int16_t y);
extern void klm_mat_mask_pixel(klm_matrix * const matrix, int16_t x, int16_t y, bool mask);

// Forward declare klm_segment because of circular refs
typedef struct klm_segment klm_segment;

typedef struct klm_matrix
{
    // Log file pointer
    FILE *logfp;

    // GPIO control pins
    uint8_t a, b, c, d, oe, r1, stb, clk;

    // Physical dimensions of the display
    int16_t width;
    int16_t height;

    // A buffer to hold the current frame
    uint8_t *display_buffer0;

    // A buffer to hold the current frame for display
    uint8_t *display_buffer1;

    // A list of available fonts and associated font-metrics
    hexfont_list *font_list;

    // Global matrix state flags
    bool on;

    // A list of virtual segments which make up the display
    klm_segment_list *segment_list;

    // Internal vars
    uint16_t _row_width;
    uint16_t _scan_row;

} klm_matrix;


/** Create a matrix object by specifying its physical characteristics */
klm_matrix * const klm_mat_create(
                            FILE *logfp,
                            uint8_t *display_buffer0,
                            uint8_t *display_buffer1,
                            uint8_t width,
                            uint8_t height,
                            uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                            uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk);

/** Clean up a matrix object */
void klm_mat_destroy(klm_matrix * const matrix);

/** Initialize a matrix object with a set of fonts and a set of segments */
void klm_mat_init(
            klm_matrix * const matrix,
            hexfont_list * const font_list,
            klm_segment_list * const segment_list);

/** Initialize a matrix object with the given font. A full-screen segment will be automatically created */
void klm_mat_simple_init(klm_matrix * const matrix, hexfont * const font);

/** Set the default full-screen segment's text content */
uint16_t klm_mat_simple_set_text(klm_matrix * const matrix, const char *text);

/** Set the animation scroll speed of the default full-screen segment in pixels per frame */
void klm_mat_simple_set_text_speed(klm_matrix * const matrix, float speed);

/** Set the position of the default full-screen segment's text */
void klm_mat_simple_set_text_position(klm_matrix * const matrix, float text_pos);

/** Start animation of matrix content */
void klm_mat_simple_start(klm_matrix * const matrix);

/** Stop animation of matrix content */
void klm_mat_simple_stop(klm_matrix * const matrix);

/** Reverse the matrix display */
void klm_mat_simple_reverse(klm_matrix * const matrix);

/** Drive animation */
void klm_mat_tick(klm_matrix * const matrix);

/** Query whether or not the given pixel has been set */
bool klm_mat_is_pixel_set(klm_matrix * const matrix, int16_t x, int16_t y);

/** Clear the entire matrix */
void klm_mat_clear(klm_matrix * const matrix);

/** Switch off matrix display altogether */
void klm_mat_on(klm_matrix * const matrix);

/** Switch on matrix display */
void klm_mat_off(klm_matrix * const matrix);

/** Print a representation of the display buffer to the console */
void klm_mat_dump_buffer(klm_matrix * const matrix);

// Inline funtions
// ----------------------------------------------------------------------------

/** Copy the buffer0 to buffer1 */
inline void klm_mat_swap_buffers(klm_matrix * const matrix) {
#ifndef KLM_NO_DOUBLE_BUFFER
    uint8_t *tmp = matrix->display_buffer1;
    matrix->display_buffer1 = matrix->display_buffer0;
    matrix->display_buffer0 = tmp;
#endif
}

/** Clear a region of the matrix */
inline void klm_mat_clear_region(
                    klm_matrix * const matrix,
                    int16_t x, int16_t y,
                    uint16_t w, uint16_t h)
{
    int16_t bx, by;
    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t _x = x + bx;
            int16_t _y = y + by;

            klm_mat_clear_pixel(matrix, _x, _y);
        }
    }
}

/** Clear a region of the matrix */
inline void klm_mat_mask_region(
                    klm_matrix * const matrix,
                    int16_t x, int16_t y,
                    uint16_t w, uint16_t h,
                    bool mask)
{
    int16_t bx, by;
    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t _x = x + bx;
            int16_t _y = y + by;

            klm_mat_mask_pixel(matrix, _x, _y, mask);
        }
    }
}

/** Set a region of pixels from a source sprite array */
inline void klm_mat_render_sprite(
                    klm_matrix * const matrix,
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
                klm_mat_set_pixel(matrix, _x, _y);
            }
            else {
                klm_mat_clear_pixel(matrix, _x, _y);
            }
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif // __KONKER_LED_MATRIX_H__