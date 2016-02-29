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

#include <tinyutf8.h>
#include "kulm_segment.h"
#include "kulm_matrix.h"

// Symbolic constants
#define KULM_BYTE_WIDTH 8
#define KULM_CHARACTER_HEIGHT 6
#define KULM_CHARACTER_SPACING 1



/** Create a segment object by */
kulm_segment * const kulm_seg_create(
                                kulm_matrix * const matrix,
                                uint8_t x,
                                uint8_t y,
                                uint8_t width,
                                uint8_t height,
                                uint8_t font_index)
{
    // Allocate memory for a kulm_matrix structure and initialize all members
    kulm_segment * const segment = malloc(sizeof(kulm_segment));

    segment->matrix = matrix;
    segment->x = x;
    segment->y = y;
    segment->width = width;
    segment->height = height;

    segment->font_index = font_index;
    segment->visible = true;
    segment->paused = false;
    segment->mask = true;

    segment->text_len = 0;
    segment->text_speed = 0;
    segment->text_pos = 0;

    int i;
    for (i=0; i<KULM_TEXT_LEN; i++) {
        segment->codepoints[i] = 0x0;
    }

    segment->_row_width = (width / KULM_BYTE_WIDTH);
    segment->_text_pixel_len = 0;

    return segment;
}

void kulm_seg_destroy(kulm_segment * const seg) {
    // Free dynamically allocated memory
    free(seg);
}

/** Drive animation */
void kulm_seg_tick(kulm_segment * const seg) {
    if (seg->paused) {
        if (seg->_dirty) {
            kulm_seg_render_text(seg);
            if (seg->mask) {
                kulm_mat_mask_region(seg->matrix,
                                     seg->x, seg->y,
                                     seg->width, seg->height,
                                     seg->mask);
            }
        }
        return;
    }

    if (seg->_dirty || seg->text_speed != 0) {
        // Animate and render text
        seg->text_pos += seg->text_speed;
        if (seg->text_pos < -seg->_text_pixel_len) {
            seg->text_pos = seg->width;
        }
        else if (seg->text_pos > seg->width) {
            seg->text_pos = -seg->_text_pixel_len;
        }

        kulm_seg_render_text(seg);
        if (seg->mask) {
            kulm_mat_mask_region(seg->matrix,
                                 seg->x, seg->y,
                                 seg->width, seg->height,
                                 seg->mask);
        }
    }
}

/** Clear a particular segment */
void kulm_seg_clear(kulm_segment * const seg) {
    kulm_mat_clear_region(seg->matrix, seg->x, seg->y, seg->width, seg->height);
}

/** Add the given segment to the rendering loop */
void kulm_seg_show(kulm_segment * const seg) {
    seg->visible = true;
    seg->_dirty = true;
}

/** Remove the given segment from the rendering loop */
void kulm_seg_hide(kulm_segment * const seg) {
    kulm_seg_clear(seg);
    seg->visible = false;
    seg->_dirty = true;
}

/** Start animation of the given segment */
void kulm_seg_start(kulm_segment * const seg) {
    seg->paused = false;
    seg->_dirty = true;
}

/** Stop animation of the given segment */
void kulm_seg_stop(kulm_segment * const seg) {
    seg->paused = true;
    seg->_dirty = false;
}

/** Set the segment's text content */
uint16_t kulm_seg_set_text(kulm_segment *seg, const char * const text) {
    //[TODO: should the codepoints buffer by dynamically allocated?]
    seg->text_len = tinyutf8_strlen(text);
    if (seg->text_len > KULM_TEXT_LEN) {
        seg->text_len = KULM_TEXT_LEN;
    }

    size_t i=0, cnt=0;
    while (cnt < seg->text_len) {
        seg->codepoints[cnt] = tinyutf8_next_codepoint(text, &i);
        cnt++;
    }

    seg->_text_pixel_len = kulm_seg_get_text_pixel_len(seg);
    seg->_dirty = true;

    return seg->_text_pixel_len;
}

/** Set the animation scroll speed of the segment in pixels per frame */
void kulm_seg_set_text_speed(kulm_segment *seg, float speed) {
    seg->text_speed = speed;
    seg->_dirty = true;
}

/** Set the position of the segment's text */
void kulm_seg_set_text_position(kulm_segment * const seg, float text_pos) {
    seg->text_pos = text_pos;
    seg->_dirty = true;
}

/** Reverse the segment */
void kulm_seg_reverse(kulm_segment * const seg) {
    seg->mask = !seg->mask;
}

/** Render the segment's text */
void kulm_seg_render_text(kulm_segment *seg) {
    uint16_t width_accum = 0;
    hexfont * const font =
        hexfont_list_get_nth(seg->matrix->font_list, seg->font_index);

    int16_t i;
    for (i=0; i<seg->text_len; i++) {
        int16_t _x = (seg->x + (int16_t)seg->text_pos + width_accum);
        hexfont_character * const c = hexfont_get(font, seg->codepoints[i]);
        if (c == NULL) {
            continue;
        }

        if (_x < seg->x + seg->width) {
            kulm_mat_render_sprite(seg->matrix,
                                   c,
                                   _x, seg->y,
                                   seg->x, seg->y,
                                   seg->x + seg->width, seg->y + seg->height);
        }
        width_accum +=
            (c->width + KULM_CHARACTER_SPACING);
    }
}

uint16_t kulm_seg_get_text_pixel_len(kulm_segment * const seg) {
    uint16_t ret = 0;
    hexfont * const font =
        hexfont_list_get_nth(seg->matrix->font_list, seg->font_index);

    int16_t i;
    for (i=0; i<seg->text_len; i++) {
        hexfont_character * const c = hexfont_get(font, seg->codepoints[i]);
        if (c == NULL) {
            continue;
        }

        ret +=
            (c->width + KULM_CHARACTER_SPACING);
    }
    return ret;
}
