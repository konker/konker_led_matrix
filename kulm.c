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
#include <stdbool.h>
#include <string.h>
#include <math.h>

#ifdef ARDUINO
#  include <Arduino.h>
#else
#  include <wiringPi.h>
#  include <wiringShift.h>
// Why aren't these in wiringPi?
#  define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#  define bitSet(value, bit) ((value) |= (1UL << (bit)))
#  define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#  define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

#include "kulm.h"

// Macros for convenience
#define KULM_ROW_OFFSET(matrix, y) (matrix->row_width*y)
#define KULM_BUF_OFFSET(matrix, x, y) (KULM_ROW_OFFSET(matrix, y)+x/8)
#define KULM_BUF_INDEX(x, y, w) (y*w + x)/KULM_BYTE_WIDTH
#define KULM_GET_PIXEL8(buf, x, y, w) buf[KULM_BUF_INDEX(x, y, w)];

// Symbolic constants
#define KULM_BYTE_WIDTH 8
#define KULM_CHARACTER_HEIGHT 6
#define KULM_CHARACTER_SPACING 1


int kulm_begin() {
#ifdef ARDUINO
    return 0;
#else
    return wiringPiSetup();
#endif
}

/**
 * Create a new matrix
 *
 * @param display_buffer  A uint8_t array of length width * height to hold the contents of the matrix display
 * @param width  The total width of the matrix, or matrices, in "pixels"
 * @param height The total height of the matrix, or matrices, in "pixels"
 *
 * @return  A pointer to a newly initialized matrix stucture
 */
kulm_matrix * const kulm_create(uint8_t *display_buffer, uint8_t width, uint8_t height, char *font[], char *font_metrics) {
    int i;

    // Allocate memory for a kulm_matrix structure and initialize all members
    kulm_matrix * const matrix = malloc(sizeof(kulm_matrix));

    matrix->width = width;
    matrix->height = height;
    matrix->display_buffer = display_buffer;
    matrix->font = font;
    matrix->font_metrics = font_metrics;
    matrix->row_width = (width / KULM_BYTE_WIDTH);

    matrix->text1_len = 0;
    matrix->text1_speed = 0;
    matrix->text1_pos = 0;
    matrix->text1_pixel_len = 0;
    for (i=0; i<KULM_TEXT1_LEN; i++) {
        matrix->text1[i] = 0x0;
    }

    matrix->text2_len = 0;
    matrix->text2_speed = 0;
    matrix->text2_pos = 0;
    matrix->text2_pixel_len = 0;
    for (i=0; i<KULM_TEXT2_LEN; i++) {
        matrix->text2[i] = 0x0;
    }

    matrix->mask = 0xff;
    matrix->on = true;
    matrix->paused = false;
    matrix->scan_row = 0;

    return matrix;
}

void kulm_destroy(kulm_matrix * const matrix) {
    // Free dynamically allocated memory
    free(matrix);
}

void kulm_init(kulm_matrix * const matrix,
              uint8_t a, uint8_t b, uint8_t c, uint8_t d,
              uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk)
{
    matrix->a = a;
    matrix->b = b;
    matrix->c = c;
    matrix->d = d;
    matrix->oe = oe;
    matrix->r1 = r1;
    matrix->stb = stb;
    matrix->clk = clk;

    pinMode(matrix->a, OUTPUT);
    pinMode(matrix->b, OUTPUT);
    pinMode(matrix->c, OUTPUT);
    pinMode(matrix->d, OUTPUT);
    pinMode(matrix->oe, OUTPUT);
    pinMode(matrix->r1, OUTPUT);
    pinMode(matrix->clk, OUTPUT);
    pinMode(matrix->stb, OUTPUT);
}

void kulm_scan(kulm_matrix *matrix) {
    if (!matrix->on) return;

    // Process each 8-pixel byte in the row
    uint8_t offset = KULM_ROW_OFFSET(matrix, matrix->scan_row);

    // Process the row in reverse order
    int16_t x8;
    for (x8=matrix->row_width-1; x8>=0; x8--) {
        uint8_t pixel8 = matrix->display_buffer[offset + x8];

        // Apply the mask
        pixel8 ^= matrix->mask;

        // Write each pixel in the byte, in reverse order
        shiftOut(matrix->r1, matrix->clk, MSBFIRST, pixel8);
    }

    // Disable display
    digitalWrite(matrix->oe, HIGH);

    // Display the rows in reverse order
    uint16_t display_row = (matrix->height - 1 - matrix->scan_row);

    // Select row
    digitalWrite(matrix->a, (display_row & 0x01));
    digitalWrite(matrix->b, (display_row & 0x02));
    digitalWrite(matrix->c, (display_row & 0x04));
    digitalWrite(matrix->d, (display_row & 0x08));

    // Latch data
    digitalWrite(matrix->stb, LOW);
    digitalWrite(matrix->stb, HIGH);
    digitalWrite(matrix->stb, LOW);

    // Enable display
    digitalWrite(matrix->oe, LOW);

    // Next row, wrap around at the bottom
    matrix->scan_row = (matrix->scan_row + 1) % matrix->height;
}

void kulm_clear(kulm_matrix *matrix) {
    int16_t i;
    for (i=0; i<(matrix->height*matrix->row_width); i++) {
        matrix->display_buffer[i] = 0x00;
    }
}

void kulm_set_pixel(kulm_matrix *matrix, int16_t x, int16_t y) {
    bitWrite(matrix->display_buffer[KULM_BUF_OFFSET(matrix, x, y)], x % KULM_BYTE_WIDTH, 1);
}

void kulm_clear_pixel(kulm_matrix *matrix, int16_t x, int16_t y) {
    bitWrite(matrix->display_buffer[KULM_BUF_OFFSET(matrix, x, y)], x % KULM_BYTE_WIDTH, 0);
}

void kulm_set_region(kulm_matrix *matrix, char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    int16_t by, bx;

    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t _x = x+bx;
            int16_t _y = y+by;
            if (_x >= matrix->width || _x < 0) {
                continue;
            }

            uint8_t pixel8 = KULM_GET_PIXEL8(buf, bx, by, w);
            uint8_t b = (bx % KULM_BYTE_WIDTH);

            if ((pixel8 << b) & 0x80) {
                kulm_set_pixel(matrix, _x, _y);
            }
            else {
                kulm_clear_pixel(matrix, _x, _y);
            }
        }
    }
}

void kulm_clear_region(kulm_matrix *matrix, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    int16_t by, bx;

    for (by=0; by<h; by++) {
        for (bx=0; bx<w; bx++) {
            int16_t X = x+bx;
            kulm_clear_pixel(matrix, X, y+by);
        }
    }
}

void kulm_start(kulm_matrix *matrix) {
    matrix->paused = false;
}

void kulm_stop(kulm_matrix *matrix) {
    matrix->paused = true;
}

void kulm_on(kulm_matrix *matrix) {
    matrix->on = true;
}

void kulm_off(kulm_matrix *matrix) {
    matrix->on = false;
    digitalWrite(matrix->oe, HIGH);
}

void kulm_reverse(kulm_matrix *matrix) {
    matrix->mask = ~matrix->mask;
}

void kulm_write_char(kulm_matrix *matrix, int16_t x, int16_t y, char c) {
    kulm_set_region(
            matrix,
            matrix->font[(unsigned char)(c)],
            x, y,
            KULM_BYTE_WIDTH,KULM_CHARACTER_HEIGHT);
}

uint16_t kulm_set_text1(kulm_matrix *matrix, const char *text) {
    strcpy(matrix->text1, text);
    matrix->text1_len = strlen(matrix->text1);
    matrix->text1_pixel_len = kulm_get_text1_pixel_len(matrix);
    return matrix->text1_pixel_len;
}

void kulm_set_text1_speed(kulm_matrix *matrix, float speed) {
    matrix->text1_speed = speed;
}

void kulm_render_text1(kulm_matrix *matrix, int16_t x_offset, int16_t y_offset) {
    uint16_t width_accum = 0;
    int16_t i;

    for (i=0; i<matrix->text1_len; i++) {
        int16_t _x = x_offset + width_accum;
        if (_x < matrix->width) {
            kulm_set_region(
                    matrix,
                    matrix->font[(unsigned char)(matrix->text1[i])],
                    _x, y_offset,
                    KULM_BYTE_WIDTH, KULM_CHARACTER_HEIGHT);
        }
        width_accum +=
            (matrix->font_metrics[(unsigned char)(matrix->text1[i])] + KULM_CHARACTER_SPACING);
    }
}

uint16_t kulm_get_text1_pixel_len(kulm_matrix *matrix) {
    uint16_t ret = 0;
    int16_t i;

    for (i=0; i<matrix->text1_len; i++) {
        ret +=
            (matrix->font_metrics[(unsigned char)(matrix->text1[i])] + KULM_CHARACTER_SPACING);
    }
    return ret;
}

uint16_t kulm_set_text2(kulm_matrix *matrix, const char *text) {
    strcpy(matrix->text2, text);
    matrix->text2_len = strlen(matrix->text2);
    matrix->text2_pixel_len = kulm_get_text2_pixel_len(matrix);
    return matrix->text2_pixel_len;
}

void kulm_set_text2_speed(kulm_matrix *matrix, float speed) {
    matrix->text2_speed = speed;
}

void kulm_render_text2(kulm_matrix *matrix, int16_t x_offset, int16_t y_offset) {
    uint16_t width_accum = 0;
    int16_t i;

    for (i=0; i<matrix->text2_len; i++) {
        int16_t _x = x_offset + width_accum;
        if (_x < matrix->width) {
            kulm_set_region(
                    matrix,
                    matrix->font[(unsigned char)(matrix->text2[i])],
                    _x, y_offset,
                    KULM_BYTE_WIDTH, KULM_CHARACTER_HEIGHT);
        }
        width_accum +=
            (matrix->font_metrics[(unsigned char)(matrix->text2[i])] + KULM_CHARACTER_SPACING);
    }
}

uint16_t kulm_get_text2_pixel_len(kulm_matrix *matrix) {
    uint16_t ret = 0;
    int16_t i;

    for (i=0; i<matrix->text2_len; i++) {
        ret +=
            (matrix->font_metrics[(unsigned char)(matrix->text2[i])] + KULM_CHARACTER_SPACING);
    }
    return ret;
}

void kulm_tick(kulm_matrix *matrix) {
    if (matrix->paused) return;

    // Animate and render text1
    matrix->text1_pos -= matrix->text1_speed;
    if (matrix->text1_pos < -1*matrix->text1_pixel_len) {
        matrix->text1_pos = matrix->width;
        kulm_clear_region(matrix, 0,KULM_TEXT1_Y, matrix->width,KULM_TEXT1_HEIGHT);
    }
    kulm_render_text1(matrix, round(matrix->text1_pos), KULM_TEXT1_Y);

    // Animate and render text2
    matrix->text2_pos -= matrix->text2_speed;
    if (matrix->text2_pos < -1*matrix->text2_pixel_len) {
        matrix->text2_pos = matrix->width;
        kulm_clear_region(matrix, 0,KULM_TEXT2_Y, matrix->width,KULM_TEXT2_HEIGHT);
    }
    kulm_render_text2(matrix, round(matrix->text2_pos), KULM_TEXT2_Y);

}

