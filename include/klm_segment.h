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

#ifndef __KONKER_LED_SEGMENT_H__
#define __KONKER_LED_SEGMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define KLM_TEXT_LEN 64

// Forward declare klm_matrix because of circular refs
typedef struct klm_matrix klm_matrix;

typedef struct klm_segment
{
    klm_matrix * matrix;

    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;

    uint8_t  font_index;
    bool     visible;
    bool     paused;
    bool     reverse;

    uint32_t codepoints[KLM_TEXT_LEN];
    size_t   text_len;
    char     const * text;
    float    text_hspeed;
    float    text_vspeed;
    float    text_hpos;
    float    text_vpos;

    uint16_t _row_width;
    uint16_t _text_pixel_width;
    uint16_t _text_pixel_height;
    bool     _dirty;

} klm_segment;

/** Create a virtual segment object */
klm_segment * const klm_seg_create(
                                klm_matrix * const matrix,
                                uint8_t x,
                                uint8_t y,
                                uint16_t width,
                                uint16_t height,
                                uint8_t font_index);

/** Clean up a virtual segment object */
void klm_seg_destroy(klm_segment * const seg);

/** Drive animation */
void klm_seg_tick(klm_segment * const seg);

/** Clear a particular segment */
void klm_seg_clear(klm_segment * const seg);

/** Add the given segment to the rendering loop */
void klm_seg_show(klm_segment * const seg);

/** Remove the given segment from the rendering loop */
void klm_seg_hide(klm_segment * const seg);

/** Start animation of the given segment */
void klm_seg_start(klm_segment * const seg);

/** Stop animation of the given segment */
void klm_seg_stop(klm_segment * const seg);

/** Set the segment's text content */
void klm_seg_set_text(klm_segment * const seg, const char *text);

/** Clear the buffer of a particular segment */
void klm_seg_clear_text(klm_segment * const seg);

/** Set the animation scroll speed of the segment in pixels per frame */
void klm_seg_set_text_speed(klm_segment * const seg, float hspeed, float vspeed);

/** Set the position of the segment's text */
void klm_seg_set_text_position(klm_segment * const seg, float text_hpos, float text_vpos);

/** Centre the segment's text */
void klm_seg_center_text(klm_segment * const seg, const bool h, const bool v);

/** Query the center coordinates for the segment's text */
void klm_seg_query_center_text(klm_segment * const seg, float *h, float *v);

/** Reverse the segment */
void klm_seg_reverse(klm_segment * const seg);

/** Render the segment's text */
void klm_seg_render_text(klm_segment * const seg);

/** Helpers */
uint16_t klm_seg_get_text_pixel_width(klm_segment * const seg);
uint16_t klm_seg_get_text_pixel_height(klm_segment * const seg);

#ifdef __cplusplus
}
#endif

#endif // __KONKER_LED_SEGMENT_H__
