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

#ifndef __KONKER_ULTRATHIN_LED_MATRIX_H__
#define __KONKER_ULTRATHIN_LED_MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define KULM_TEXT1_LEN 64
#define KULM_TEXT2_LEN 64

#define KULM_TEXT1_HEIGHT 8
#define KULM_TEXT2_HEIGHT 8

#define KULM_TEXT1_Y 0
#define KULM_TEXT2_Y 8


typedef struct kulm_matrix
{
    uint8_t  a, b, c, d, oe, r1, stb, clk;

    int16_t width;
    int16_t height;

    uint8_t  *display_buffer;
    char     **font;
    char     *font_metrics;
    uint8_t  mask;
    bool     on;
    bool     paused;

    char     text1[KULM_TEXT1_LEN];
    uint16_t text1_len;
    float    text1_speed;
    float    text1_pos;
    float    text1_pixel_len;

    char     text2[KULM_TEXT2_LEN];
    uint16_t text2_len;
    float    text2_speed;
    float    text2_pos;
    float    text2_pixel_len;

    uint16_t row_width;
    uint16_t scan_row;
} kulm_matrix;


int kulm_begin();
kulm_matrix * const kulm_create(uint8_t *display_buffer, uint8_t width, uint8_t height, char **font, char *font_metrics);
void kulm_destroy(kulm_matrix * const matrix);
void kulm_init(kulm_matrix * const matrix,
              uint8_t a, uint8_t b, uint8_t c, uint8_t d,
              uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk);
void kulm_scan(kulm_matrix * const matrix);
void kulm_clear(kulm_matrix * const matrix);
inline void kulm_set_pixel(kulm_matrix * const matrix, int16_t x, int16_t y);
inline void kulm_clear_pixel(kulm_matrix * const matrix, int16_t x, int16_t y);
void kulm_set_region(kulm_matrix * const matrix, char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h);
void kulm_clear_region(kulm_matrix * const matrix, int16_t x, int16_t y, uint16_t w, uint16_t h);
void kulm_start(kulm_matrix * const matrix);
void kulm_stop(kulm_matrix * const matrix);
void kulm_on(kulm_matrix * const matrix);
void kulm_off(kulm_matrix * const matrix);
void kulm_reverse(kulm_matrix * const matrix);
void kulm_write_char(kulm_matrix * const matrix, int16_t x, int16_t y, char c);

uint16_t kulm_set_text1(kulm_matrix * const matrix, const char *text1);
void kulm_set_text1_speed(kulm_matrix * const matrix, float speed);
uint16_t kulm_get_text1_pixel_len(kulm_matrix * const matrix);
void kulm_render_text1(kulm_matrix * const matrix, int16_t x_offset, int16_t y_offset);

uint16_t kulm_set_text2(kulm_matrix * const matrix, const char *text2);
void kulm_set_text2_speed(kulm_matrix * const matrix, float speed);
uint16_t kulm_get_text2_pixel_len(kulm_matrix * const matrix);
void kulm_render_text2(kulm_matrix * const matrix, int16_t x_offset, int16_t y_offset);
void kulm_tick(kulm_matrix * const matrix);

#ifdef __cplusplus
}
#endif

#endif // __KONKER_ULTRATHIN_LED_MATRIX_H__
