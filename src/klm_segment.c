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

#include <tinyutf8.h>
#include "klm_segment.h"
#include "klm_matrix.h"

// Symbolic constants
#define KLM_BYTE_WIDTH 8
#define KLM_CHARACTER_SPACING 1



/** Create a segment object by */
klm_segment * const klm_seg_create(
                                klm_matrix * const matrix,
                                uint8_t x,
                                uint8_t y,
                                uint16_t width,
                                uint16_t height,
                                uint8_t font_index)
{
    // Allocate memory for a klm_matrix structure and initialize all members
    klm_segment * const segment = malloc(sizeof(klm_segment));

    segment->matrix = matrix;
    segment->x = x;
    segment->y = y;
    segment->width = width;
    segment->height = height;

    segment->font_index = font_index;
    segment->visible = true;
    segment->paused = false;
    segment->reverse = false;

    segment->text_len = 0;
    segment->text_speed = 0;
    segment->text_pos = 0;

    int i;
    for (i=0; i<KLM_TEXT_LEN; i++) {
        segment->codepoints[i] = 0x0;
    }

    segment->_row_width = (uint16_t)(width / KLM_BYTE_WIDTH);
    segment->_text_pixel_len = 0;
    segment->_dirty = false;

    return segment;
}

void klm_seg_destroy(klm_segment * const seg) {
    // Free dynamically allocated memory
    free(seg);
}

/** Drive animation */
void klm_seg_tick(klm_segment * const seg) {
    if (!seg->visible) {
        return;
    }

    if (seg->paused) {
        if (seg->_dirty) {
            klm_seg_render_text(seg);
            if (seg->reverse) {
                klm_mat_mask_region(seg->matrix,
                                     seg->x, seg->y,
                                     seg->width, seg->height,
                                     seg->reverse);
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

        klm_seg_render_text(seg);
        if (seg->reverse) {
            klm_mat_mask_region(seg->matrix,
                                 seg->x, seg->y,
                                 seg->width, seg->height,
                                 seg->reverse);
        }
    }
    if (seg->_dirty) {
        klm_seg_render_text(seg);
        if (seg->reverse) {
            klm_mat_mask_region(seg->matrix,
                                 seg->x, seg->y,
                                 seg->width, seg->height,
                                 seg->reverse);
        }
    }
}

/** Clear a particular segment */
void klm_seg_clear(klm_segment * const seg) {
    klm_mat_clear_region(seg->matrix, seg->x, seg->y, seg->width, seg->height);
}

/** Add the given segment to the rendering loop */
void klm_seg_show(klm_segment * const seg) {
    seg->visible = true;
    seg->_dirty = true;
}

/** Remove the given segment from the rendering loop */
void klm_seg_hide(klm_segment * const seg) {
    klm_seg_clear(seg);
    seg->visible = false;
    seg->_dirty = true;
}

/** Set the segment's text content */
uint16_t klm_seg_set_text(klm_segment *seg, const char * const text) {
    //[TODO: should the codepoints buffer by dynamically allocated?]
    seg->text_len = tinyutf8_strlen(text);
    if (seg->text_len > KLM_TEXT_LEN) {
        seg->text_len = KLM_TEXT_LEN;
    }

    size_t i=0, cnt=0;
    while (cnt < seg->text_len) {
        seg->codepoints[cnt] = tinyutf8_next_codepoint(text, &i);
        cnt++;
    }

    seg->_text_pixel_len = klm_seg_get_text_pixel_len(seg);
    seg->_dirty = true;

    return seg->_text_pixel_len;
}

/** Set the animation scroll speed of the segment in pixels per frame */
void klm_seg_set_text_speed(klm_segment *seg, float speed) {
    seg->text_speed = speed;
    seg->_dirty = true;
}

/** Start animation of the given segment */
void klm_seg_start(klm_segment * const seg) {
    seg->paused = false;
    seg->_dirty = true;
}

/** Stop animation of the given segment */
void klm_seg_stop(klm_segment * const seg) {
    seg->paused = true;
    seg->_dirty = false;
}

/** Set the position of the segment's text */
void klm_seg_set_text_position(klm_segment * const seg, float text_pos) {
    seg->text_pos = text_pos;
    seg->_dirty = true;
}

/** Reverse the segment */
void klm_seg_reverse(klm_segment * const seg) {
    seg->reverse = !seg->reverse;
}

/** Render the segment's text */
void klm_seg_render_text(klm_segment *seg) {
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
            klm_mat_render_sprite(seg->matrix,
                                   c,
                                   _x, seg->y,
                                   seg->x, seg->y,
                                   seg->x + seg->width, seg->y + seg->height);
        }
        width_accum +=
            (c->width + KLM_CHARACTER_SPACING);
    }
}

uint16_t klm_seg_get_text_pixel_len(klm_segment * const seg) {
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
            (c->width + KLM_CHARACTER_SPACING);
    }
    return ret;
}
