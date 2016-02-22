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
 * konker_ultrathin_led_matrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with konker_ultrathin_led_matrix.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "kulm_matrix.h"
#include "kulm_segment.h"

static void _kulm_mat_sanity_check(kulm_matrix * const matrix);

extern inline void kulm_mat_set_pixel(kulm_matrix * const matrix, int16_t x, int16_t y);
extern inline void kulm_mat_clear_pixel(kulm_matrix * const matrix, int16_t x, int16_t y);
extern inline void kulm_mat_clear_region(kulm_matrix * const matrix, int16_t x, int16_t y, uint16_t w, uint16_t h);
extern inline void kulm_mat_render_sprite(
                    kulm_matrix * const matrix,
                    hexfont_character * const sprite,
                    int16_t x, int16_t y,
                    int16_t clip_x0, int16_t clip_y0,
                    int16_t clip_x1, int16_t clip_y1);
extern inline void kulm_mat_scan(kulm_matrix * const matrix);
extern inline void kulm_mat_swap_buffers(kulm_matrix * const matrix);

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
                            FILE *logfp,
                            uint8_t *display_buffer0,
                            uint8_t *display_buffer1,
                            uint8_t width,
                            uint8_t height,
                            uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                            uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk)
{
    // Allocate memory for a kulm_matrix structure and initialize all members
    kulm_matrix * const matrix = malloc(sizeof(kulm_matrix));

    matrix->logfp = logfp;
    matrix->width = width;
    matrix->height = height;
    matrix->display_buffer0 = display_buffer0;
    matrix->display_buffer1 = display_buffer1;
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
    matrix->_scan_row = 0;

    matrix->mask = 0xff;

    return matrix;
}

/** Clean up a matrix object */
void kulm_mat_destroy(kulm_matrix * const matrix) {
    // Clean up the font list
    hexfont_list_destroy(matrix->font_list);

    // Clean up the font list
    kulm_segment_list_destroy(matrix->segment_list);

    // Free dynamically allocated memory for the matrix itself
    free(matrix);
}

/** Initialize a matrix object with a set of fonts and a set of segments */
void kulm_mat_init(kulm_matrix * const matrix,
                   hexfont_list *font_list,
                   kulm_segment_list *segment_list)
{
    matrix->font_list = font_list;
    matrix->segment_list = segment_list;

#ifndef KULM_NON_GPIO_MACHINE
    // Initilize pin modes
    pinMode(matrix->a, OUTPUT);
    pinMode(matrix->b, OUTPUT);
    pinMode(matrix->c, OUTPUT);
    pinMode(matrix->d, OUTPUT);
    pinMode(matrix->oe, OUTPUT);
    pinMode(matrix->r1, OUTPUT);
    pinMode(matrix->clk, OUTPUT);
    pinMode(matrix->stb, OUTPUT);
#endif

    kulm_mat_clear(matrix);

    _kulm_mat_sanity_check(matrix);
}

/** Initialize a matrix object with the given font.
    A full-screen segment will be automatically created */
void kulm_mat_simple_init(kulm_matrix * const matrix, hexfont * const font) {
    // Create a font list containing the given font
    hexfont_list * const _default_font_list = hexfont_list_create(font);

    // Create a single full-screen segment
    kulm_segment * const _default_segment =
                                kulm_seg_create(matrix,
                                                0, 0,
                                                matrix->width,
                                                matrix->height,
                                                0);

    // Create a segment list with one item
    kulm_segment_list * const _default_segment_list =
                                    kulm_segment_list_create(_default_segment);

    // Initialize the matrix with the default font list and segment list
    kulm_mat_init(matrix,
                  _default_font_list,
                  _default_segment_list);
}

/** Set the default full-screen segment's text content */
uint16_t kulm_mat_simple_set_text(kulm_matrix * const matrix, const char *text) {
    return kulm_seg_set_text(matrix->segment_list->item, text);
}

/** Set the animation scroll speed of the default full-screen segment in pixels per frame */
void kulm_mat_simple_set_text_speed(kulm_matrix * const matrix, float speed) {
    kulm_seg_set_text_speed(matrix->segment_list->item, speed);
}

/** Set the position of the default full-screen segment's text */
void kulm_mat_simple_set_text_position(kulm_matrix * const matrix, float text_pos) {
    kulm_seg_set_text_position(matrix->segment_list->item, text_pos);
}

/** Start animation of matrix content */
void kulm_mat_simple_start(kulm_matrix *matrix) {
    kulm_seg_start(matrix->segment_list->item);
}

/** Stop animation of matrix content */
void kulm_mat_simple_stop(kulm_matrix *matrix) {
    kulm_seg_stop(matrix->segment_list->item);
}

/** Drive animation */
void kulm_mat_tick(kulm_matrix *matrix) {
    kulm_segment_list *iter = matrix->segment_list;
    while (iter) {
        kulm_seg_tick(iter->item);
        iter = iter->next;
    }

    kulm_mat_swap_buffers(matrix);
}

/** Query whether or not the given pixel has been set */
bool kulm_mat_is_pixel_set(kulm_matrix * const matrix, int16_t x, int16_t y) {
    size_t p = KULM_BUF_OFFSET(matrix, x, y);
#ifndef KULM_NO_DOUBLE_BUFFER
    if (bitRead(matrix->display_buffer1[p], x % KULM_BYTE_WIDTH) == KULM_ON) {
        return true;
    }
#else
    if (bitRead(matrix->display_buffer0[p], x % KULM_BYTE_WIDTH) == KULM_ON) {
        return true;
    }
#endif
    return false;
}

/** Clear the entire matrix */
void kulm_mat_clear(kulm_matrix *matrix) {
    int16_t i;
#ifndef KULM_NO_DOUBLE_BUFFER
    for (i=0; i<(matrix->height*matrix->_row_width); i++) {
        matrix->display_buffer1[i] = KULM_OFF_BYTE;
    }
#endif
    for (i=0; i<(matrix->height*matrix->_row_width); i++) {
        matrix->display_buffer0[i] = KULM_OFF_BYTE;
    }
}

/** Switch off matrix display altogether */
void kulm_mat_on(kulm_matrix *matrix) {
    matrix->on = true;
}

/** Switch on matrix display */
void kulm_mat_off(kulm_matrix *matrix) {
    matrix->on = false;
#ifndef KULM_NON_GPIO_MACHINE
    digitalWrite(matrix->oe, HIGH);
#endif
}

/** Reverse the matrix display */
void kulm_mat_reverse(kulm_matrix * const matrix) {
    matrix->mask = ~matrix->mask;
}

void kulm_mat_dump_buffer(kulm_matrix * const matrix) {
    int16_t i;
    for (i=0; i<matrix->height*matrix->_row_width; i++) {
        KULM_LOG(matrix, "%02x ", matrix->display_buffer0[i]);
    }
    KULM_LOG(matrix, "\n");
#ifndef KULM_NO_DOUBLE_BUFFER
    for (i=0; i<matrix->height*matrix->_row_width; i++) {
        KULM_LOG(matrix, "%02x ", matrix->display_buffer1[i]);
    }
    KULM_LOG(matrix, "\n");
#endif

    int16_t x, y;
    for (y=0; y<matrix->height; y++) {
        for (x=0; x<matrix->width; x++) {
            if (kulm_mat_is_pixel_set(matrix, x, y)) {
                KULM_LOG(matrix, "# ");
            }
            else {
                KULM_LOG(matrix, ". ");
            }
        }
        KULM_LOG(matrix, "\n");
    }
    KULM_LOG(matrix, "\n");
}

static void _kulm_mat_sanity_check(kulm_matrix * const matrix) {
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

