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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "klm_matrix.h"
#include "klm_segment.h"

static void _klm_mat_sanity_check(klm_matrix * const matrix);

static inline void klm_mat_clear_region(klm_matrix * const matrix, int16_t x, int16_t y, uint16_t w, uint16_t h);
static inline void klm_mat_mask_region(klm_matrix * const matrix, int16_t x, int16_t y, uint16_t w, uint16_t h, bool reverse);
static inline void klm_mat_render_sprite(
                    klm_matrix * const matrix,
                    hexfont_character * const sprite,
                    int16_t x, int16_t y,
                    int16_t clip_x0, int16_t clip_y0,
                    int16_t clip_x1, int16_t clip_y1);
static inline void klm_mat_swap_buffers(klm_matrix * const matrix);

/**
 * Call any necessary one-time initialization
 *
 * @return  True if the system was initialized correctly
 */
bool klm_mat_begin() {
    // Initialize WirinPi if necessary
#ifdef KLM_WIRING_PI
    if (wiringPiSetup()) {
        return false;
    }
#endif
    return true;
}

klm_matrix * const klm_mat_create(FILE *logfp, klm_config * const config) {
    // Allocate memory for a klm_matrix structure and initialize all members
    klm_matrix * const matrix = malloc(sizeof(klm_matrix));

    matrix->config = config;
    matrix->logfp = logfp;
    matrix->_row_width = (uint16_t)(matrix->config->width / KLM_BYTE_WIDTH);

    klm_mat_init_display_buffer(matrix);
    matrix->_dynamic_buffer = true;

    matrix->on = true;
    matrix->scan_modulation = 0;

    matrix->_scan_row = 0;
    matrix->micros_0 = 0;
    matrix->micros_1 = 0;

    return matrix;
}

/** Clean up a matrix object */
void klm_mat_destroy(klm_matrix * const matrix) {
    // Clean up the font list
    hexfont_list_destroy(matrix->font_list);

    // Clean up the font list
    klm_segment_list_destroy(matrix->segment_list);

    // If display buffer(s) are dynamically allocated, free them
    if (matrix->_dynamic_buffer) {
        free(matrix->display_buffer0);
        free(matrix->display_buffer1);
    }

    // Free dynamically allocated memory for the matrix itself
    free(matrix);
}

/** Initialize a matrix object with a set of fonts and a set of segments */
void klm_mat_init(klm_matrix * const matrix,
                  hexfont_list *font_list,
                  klm_segment_list *segment_list)
{
    matrix->font_list = font_list;
    matrix->segment_list = segment_list;

    klm_mat_init_hardware(matrix);

    klm_mat_clear(matrix);

    _klm_mat_sanity_check(matrix);
}

/** Initialize a matrix object with the given font.
    A full-screen segment will be automatically created */
void klm_mat_simple_init(klm_matrix * const matrix,
                         hexfont * const font)
{
    // Create a font list containing the given font
    hexfont_list * const _default_font_list = hexfont_list_create(font);

    // Create a single full-screen segment
    klm_segment * const _default_segment =
                                klm_seg_create(matrix,
                                                0, 0,
                                                matrix->config->width,
                                                matrix->config->height,
                                                0);

    // Create a segment list with one item
    klm_segment_list * const _default_segment_list =
                                    klm_segment_list_create(_default_segment);

    // Initialize the matrix with the default font list and segment list
    klm_mat_init(matrix,
                 _default_font_list,
                 _default_segment_list);
}

/** Set the default full-screen segment's text content */
uint16_t klm_mat_simple_set_text(klm_matrix * const matrix, const char *text) {
    return klm_seg_set_text(matrix->segment_list->item, text);
}

#ifdef KLM_NATIVE_ANIMATION
/** Set the animation scroll speed of the default full-screen segment in pixels per frame */
void klm_mat_simple_set_text_speed(klm_matrix * const matrix, float speed) {
    klm_seg_set_text_speed(matrix->segment_list->item, speed);
}

/** Start animation of matrix content */
void klm_mat_simple_start(klm_matrix *matrix) {
    klm_seg_start(matrix->segment_list->item);
}

/** Stop animation of matrix content */
void klm_mat_simple_stop(klm_matrix *matrix) {
    klm_seg_stop(matrix->segment_list->item);
}
#endif

/** Set the position of the default full-screen segment's text */
void klm_mat_simple_set_text_position(klm_matrix * const matrix, float text_pos) {
    klm_seg_set_text_position(matrix->segment_list->item, text_pos);
}

/** Reverse the matrix display */
void klm_mat_simple_reverse(klm_matrix * const matrix) {
    klm_seg_reverse(matrix->segment_list->item);
}

/** Drive animation */
void klm_mat_tick(klm_matrix *matrix) {
    KLM_NOW_MICROSECS(matrix->micros_1, matrix->now_t);

    // Delay to make loop time consistent
    int64_t period = KLM_TICK_PERIOD_MICROS
                        - (matrix->micros_1 - matrix->micros_0);

    if (period <= 0) {
        klm_mat_clear(matrix);

        klm_segment_list *iter = matrix->segment_list;
        while (iter) {
            klm_seg_tick(iter->item);
            iter = iter->next;
        }
        klm_mat_swap_buffers(matrix);

        matrix->micros_0 = matrix->micros_1;
    }
}

/** Switch off matrix display altogether */
void klm_mat_on(klm_matrix *matrix) {
    matrix->on = true;
}

/** Switch on matrix display */
void klm_mat_off(klm_matrix *matrix) {
    matrix->on = false;
#ifndef KLM_NON_GPIO_MACHINE
    digitalWrite(klm_config_get_pin(matrix->config, 'o'), HIGH);
#endif
}

/** Set the scan loop modulation */
void klm_mat_set_scan_modulation(klm_matrix * const matrix, uint16_t scan_modulation) {
    matrix->scan_modulation = scan_modulation;
}

/** Clear the entire matrix */
void klm_mat_clear(klm_matrix *matrix) {
    int16_t x, y;
    for (y=0; y<matrix->config->height; y++) {
        for (x = 0; x < matrix->config->width; x++) {
            klm_mat_clear_pixel(matrix, x, y);
        }
    }
}

void klm_mat_dump_buffer(klm_matrix * const matrix) {
    int16_t i;
    for (i=0; i<matrix->config->height*matrix->_row_width; i++) {
        KLM_LOG(matrix, "%02x ", matrix->display_buffer1[i]);
    }
    KLM_LOG(matrix, "\n");

    int16_t x, y;
    for (y=0; y<matrix->config->height; y++) {
        for (x=0; x<matrix->config->width; x++) {
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

static void _klm_mat_sanity_check(klm_matrix * const matrix) {
    // Check that segments are within the bounds of the matrix
    //[TODO]

    // Check that segments do not overlap
    //[TODO]]

    // Check that font references exist in font list
    //[TODO]

    // Check that font sizes fit within segments
    //[TODO]

    // Check that buffer0 and buffer1 have the same size
    //[TODO]
}

