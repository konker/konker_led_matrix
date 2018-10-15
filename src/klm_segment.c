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

    segment->text = strdup("");
    segment->text_len = 0;
    segment->text_hspeed = 0;
    segment->text_vspeed = 0;
    segment->text_hpos = 0;
    segment->text_vpos = 0;

    int i;
    for (i=0; i<KLM_TEXT_LEN; i++) {
        segment->codepoints[i] = 0x0;
    }

    segment->_row_width = (uint16_t)(width / KLM_BYTE_WIDTH);
    segment->_text_pixel_width = 0;
    segment->_text_pixel_height = 0;
    segment->_dirty = false;

    return segment;
}

void klm_seg_destroy(klm_segment * const seg) {
    // Free dynamically allocated memory
    free((char *)seg->text);
    free(seg);
}

/** Drive animation */
void klm_seg_tick(klm_segment * const seg) {
    if (!seg->visible) {
        return;
    }

    /*
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
    */

    if (seg->text_hspeed != 0 && !seg->paused) {
        // Animate and render text horizontally
        seg->text_hpos += seg->text_hspeed;
        if (seg->text_hpos < -seg->_text_pixel_width) {
            seg->text_hpos = seg->width;
        }
        else if (seg->text_hpos > seg->width) {
            seg->text_hpos = -seg->_text_pixel_width;
        }
    }

    if (seg->text_vspeed != 0 && !seg->paused) {
        // Animate and render text vertically
        seg->text_vpos += seg->text_vspeed;
        if (seg->text_vpos < -seg->_text_pixel_height) {
            seg->text_vpos = seg->height;
        }
        else if (seg->text_vpos > seg->width) {
            seg->text_vpos = -seg->_text_pixel_height;
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
void klm_seg_set_text(klm_segment *seg, const char * const text) {
    //[TODO: should the codepoints buffer by dynamically allocated?]
    seg->text_len = tinyutf8_strlen(text);
    if (seg->text_len > KLM_TEXT_LEN) {
        seg->text_len = KLM_TEXT_LEN;
    }

    // Decompose the text into codepoints
    size_t i=0, cnt=0;
    while (cnt < seg->text_len) {
        seg->codepoints[cnt] = tinyutf8_next_codepoint(text, &i);
        cnt++;
    }

    // Save the original text
    free((char *)seg->text);
    seg->text = strdup(text);

    seg->_text_pixel_width = klm_seg_get_text_pixel_width(seg);
    seg->_text_pixel_height = klm_seg_get_text_pixel_height(seg);
    seg->_dirty = true;

    return;
}

/** Clear the text of a particular segment */
void klm_seg_clear_text(klm_segment * const seg) {
    klm_seg_set_text(seg, "");
    klm_seg_set_text_speed(seg, 0, 0);
    klm_seg_set_text_position(seg, 0, 0);
}

/** Set the animation scroll speed of the segment in pixels per frame */
void klm_seg_set_text_speed(klm_segment *seg, float hspeed, float vspeed) {
    seg->text_hspeed = hspeed;
    seg->text_vspeed = vspeed;
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
    seg->_dirty = true;
}

/** Set the position of the segment's text */
void klm_seg_set_text_position(klm_segment * const seg, float text_hpos, float text_vpos) {
    seg->text_hpos = text_hpos;
    seg->text_vpos = text_vpos;
    seg->_dirty = true;
}

/** Center the segment's text horizontally */
void klm_seg_center_text(klm_segment * const seg) {
    int pl = klm_seg_get_text_pixel_width(seg);
    seg->text_hpos = -(pl/2 - seg->width/2);

    pl = klm_seg_get_text_pixel_height(seg);
    seg->text_vpos = -(pl/2 - seg->height/2);

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
        int16_t _x = (seg->x + (int16_t)seg->text_hpos + width_accum);
        int16_t _y = (seg->y + (int16_t)seg->text_vpos);
        hexfont_character * const c = hexfont_get(font, seg->codepoints[i]);
        if (c == NULL) {
            continue;
        }

        if (_x < seg->x + seg->width) {
            klm_mat_render_sprite(seg->matrix,
                                   c,
                                   _x, _y,
                                   seg->x, seg->y,
                                   seg->x + seg->width, seg->y + seg->height);
        }
        width_accum +=
            (c->width + KLM_CHARACTER_SPACING);
    }
}

uint16_t klm_seg_get_text_pixel_width(klm_segment * const seg) {
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

uint16_t klm_seg_get_text_pixel_height(klm_segment * const seg) {
    uint16_t ret = 0;
    hexfont * const font =
        hexfont_list_get_nth(seg->matrix->font_list, seg->font_index);

    int16_t i;
    for (i=0; i<seg->text_len; i++) {
        hexfont_character * const c = hexfont_get(font, seg->codepoints[i]);
        if (c == NULL) {
            continue;
        }

        if (c->height > ret) {
            ret = c->height;
        }
    }
    return ret;
}
