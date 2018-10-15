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
#  ifdef KLM_WIRING_PI
#    include <wiringPi.h>
#    include <wiringShift.h>
#  endif
// Why aren't these in wiringPi?
#  define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#  define bitSet(value, bit) ((value) |= (1UL << (bit)))
#  define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#  define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

#if !defined(ARDUINO) && !defined(KLM_WIRING_PI)
#define KLM_NON_GPIO_MACHINE
#endif

#include <time.h>
#include <hexfont.h>
#include <hexfont_list.h>
#include "klm_segment.h"
#include "klm_segment_list.h"
#include "klm_config.h"

// Symbolic constants
#define KLM_BYTE_WIDTH 8
#define KLM_OFF_BYTE 0x00
#define KLM_OFF 0x00
#define KLM_ON 0x01

// Macros for convenience
#define KLM_BUFFER_LEN(w, h) (size_t)(h * (w/KLM_BYTE_WIDTH))
#define KLM_ROW_OFFSET(matrix, y) (matrix->_row_width*y)
#define KLM_BUF_OFFSET(matrix, x, y) (size_t)(KLM_ROW_OFFSET(matrix, y)+x/KLM_BYTE_WIDTH)
#define KLM_LOG(matrix, ...) fprintf(matrix->logfp, __VA_ARGS__); \
                             fflush(matrix->logfp);
#define KLM_LOCK(lock) if (lock != NULL) { *lock = true; }
#define KLM_UNLOCK(lock) if (lock != NULL) { *lock = false; }

#define KLM_ONE_MILLION 1000000
#define KLM_ONE_THOUSAND 1000
#define KLM_NOW_MICROSECS(var, time_spec_var) \
        clock_gettime(CLOCK_REALTIME, &time_spec_var); \
        var =  time_spec_var.tv_sec * KLM_ONE_MILLION; \
        var += time_spec_var.tv_nsec / KLM_ONE_THOUSAND; \

// 1 thousand => 1 millisecond
#define KLM_TICK_PERIOD_MICROS 100 * KLM_ONE_THOUSAND

// Forward declare klm_segment because of circular refs
typedef struct klm_segment klm_segment;

typedef struct klm_matrix
{
    // Log file pointer
    FILE *logfp;

    klm_config *config;

    // A buffer to hold the current frame
    uint8_t *display_buffer0;

    // A buffer to hold the current frame for display
    uint8_t *display_buffer1;

    // Whether the display buffer(s) were dynamically allocated
    bool _dynamic_buffer;

    // A list of available fonts and associated font-metrics
    hexfont_list *font_list;

    // Global matrix state flags
    bool on;

    // A number to indicate the level of modulation used for "dimming"
    uint16_t scan_modulation;

    // A list of virtual segments which make up the display
    klm_segment_list *segment_list;

    // Keep track of the current scan row
    uint16_t scan_row;

    // Internal vars
    uint16_t _row_width;
    struct timespec now_t;
    int64_t micros_0;
    int64_t micros_1;

} klm_matrix;


/** Call any necessary one-time initialization */
bool klm_mat_begin();

/** Create a matrix object by specifying its physical characteristics */
klm_matrix * const klm_mat_create(FILE *logfp, klm_config * const config);

/** Clean up a matrix object */
void klm_mat_destroy(klm_matrix * const matrix);

/** Initialize a matrix object with a set of fonts and a set of segments */
void klm_mat_init(klm_matrix * const matrix,
                  hexfont_list * const font_list,
                  klm_segment_list * const segment_list);

/** Initialize a matrix object with the given font. A full-screen segment will be automatically created */
void klm_mat_simple_init(klm_matrix * const matrix,
                         hexfont * const font);

/** Set the default full-screen segment's text content */
void klm_mat_simple_set_text(klm_matrix * const matrix, const char *text);

/** Set the animation scroll speed of the default full-screen segment in pixels per frame */
void klm_mat_simple_set_text_speed(klm_matrix * const matrix, float hspeed, float vspeed);

/** Start animation of matrix content */
void klm_mat_simple_start(klm_matrix * const matrix);

/** Stop animation of matrix content */
void klm_mat_simple_stop(klm_matrix * const matrix);

/** Set the position of the default full-screen segment's text */
void klm_mat_simple_set_text_position(klm_matrix * const matrix, float text_hpos, float text_vpos);

/** Reverse the matrix display */
void klm_mat_simple_reverse(klm_matrix * const matrix);

/** Drive animation */
void klm_mat_tick(klm_matrix * const matrix);

/** Clear the entire matrix */
void klm_mat_clear(klm_matrix * const matrix);

/** Clear the text of the entire matrix */
void klm_mat_clear_text(klm_matrix * const matrix);

/** Switch off matrix display altogether */
void klm_mat_on(klm_matrix * const matrix);

/** Switch on matrix display */
void klm_mat_off(klm_matrix * const matrix);

/** Set the scan loop modulation */
void klm_mat_set_scan_modulation(klm_matrix * const matrix, uint16_t scan_modulation);


// Driver functions
// ----------------------------------------------------------------------------
/** Drive the matrix hardware */
extern void klm_mat_scan(klm_matrix * const matrix);

/** Set a pixel */
extern void klm_mat_set_pixel(klm_matrix * const matrix, int16_t x, int16_t y);

/** Clear a pixel */
extern void klm_mat_clear_pixel(klm_matrix * const matrix, int16_t x, int16_t y);

/** Apply a mask to a given pixel */
extern void klm_mat_mask_pixel(klm_matrix * const matrix, int16_t x, int16_t y, bool reverse);

/** Clear the matrix */
extern void klm_mat_clear(klm_matrix *matrix);

/** Query whether or not the given pixel has been set */
extern bool klm_mat_is_pixel_set(klm_matrix * const matrix, int16_t x, int16_t y);

/** Print a representation of the display buffer to the console */
extern void klm_mat_dump_buffer(klm_matrix * const matrix);

extern void klm_mat_init_hardware(klm_matrix * const matrix);
extern void klm_mat_init_display_buffer(klm_matrix * const matrix);

// Inline funtions
// ----------------------------------------------------------------------------
/** Copy the buffer0 to buffer1 */
static inline void klm_mat_swap_buffers(klm_matrix * const matrix) {
    uint8_t *tmp = matrix->display_buffer1;
    matrix->display_buffer1 = matrix->display_buffer0;
    matrix->display_buffer0 = tmp;
}

/** Clear a region of the matrix */
static inline void klm_mat_clear_region(
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
static inline void klm_mat_mask_region(
                    klm_matrix * const matrix,
                    int16_t x, int16_t y,
                    uint16_t w, uint16_t h,
                    bool reverse)
{
    int16_t bx, by;
    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t _x = x + bx;
            int16_t _y = y + by;

            klm_mat_mask_pixel(matrix, _x, _y, reverse);
        }
    }
}

/** Set a region of pixels from a source sprite array */
static inline void klm_mat_render_sprite(
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
