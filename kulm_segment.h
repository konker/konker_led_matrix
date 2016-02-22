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

#ifndef __KONKER_ULTRATHIN_LED_SEGMENT_H__
#define __KONKER_ULTRATHIN_LED_SEGMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define KULM_TEXT_LEN 64

// Forward declare kulm_matrix because of circular refs
typedef struct kulm_matrix kulm_matrix;

typedef struct kulm_segment
{
    kulm_matrix * matrix;

    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;

    uint8_t  font_index;
    bool     visible;
    bool     paused;

    uint32_t codepoints[KULM_TEXT_LEN];
    size_t   text_len;
    float    text_speed;
    float    text_pos;

    uint16_t _row_width;
    uint16_t _text_pixel_len;
    bool     _dirty;

} kulm_segment;

/** Create a virtual segment object */
kulm_segment * const kulm_seg_create(
                                kulm_matrix * const matrix,
                                uint8_t x,
                                uint8_t y,
                                uint8_t width,
                                uint8_t height,
                                uint8_t font_index);

/** Clean up a virtual segment object */
void kulm_seg_destroy(kulm_segment * const seg);

/** Drive animation */
void kulm_seg_tick(kulm_segment * const seg);

/** Clear a particular segment */
void kulm_seg_clear(kulm_segment * const seg);

/** Add the given segment to the rendering loop */
void kulm_seg_show(kulm_segment * const seg);

/** Remove the given segment from the rendering loop */
void kulm_seg_hide(kulm_segment * const seg);

/** Start animation of the given segment */
void kulm_seg_start(kulm_segment * const seg);

/** Stop animation of the given segment */
void kulm_seg_stop(kulm_segment * const seg);

/** Set the segment's text content */
uint16_t kulm_seg_set_text(kulm_segment * const seg, const char *text);

/** Set the animation scroll speed of the segment in pixels per frame */
void kulm_seg_set_text_speed(kulm_segment * const seg, float speed);

/** Set the position of the segment's text */
void kulm_seg_set_text_position(kulm_segment * const seg, float text_pos);

/** Render the segment's text */
void kulm_seg_render_text(kulm_segment * const seg);

uint16_t kulm_seg_get_text_pixel_len(kulm_segment * const seg);

#ifdef __cplusplus
}
#endif

#endif // __KONKER_ULTRATHIN_LED_SEGMENT_H__
