/**
 * UltrathinLEDMatrix
 *
 * Author: Konrad Markus <konker@luxvelocitas.com>
 *
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringShift.h>
#include "UltrathinLEDMatrix.h"
#include "konker_bitfont_basic.h"


struct UltrathinLEDMatrix * const ultrathin_matrix_create(uint8_t *display_buffer, uint8_t width, uint8_t height) {
    struct UltrathinLEDMatrix * const matrix =
                        malloc(sizeof(struct UltrathinLEDMatrix));

    matrix->width = width;
    matrix->height = height;
    matrix->display_buffer = display_buffer;
    matrix->row_width = (width / BYTE_WIDTH);

    matrix->text1_len = 0;
    matrix->text1_speed = 0;
    matrix->text1_pos = 0;
    matrix->text1_pixel_len = 0;
    for (int i=0; i<TEXT1_LEN; i++) {
        matrix->text1[i] = 0x0;
    }

    matrix->text2_len = 0;
    matrix->text2_speed = 0;
    matrix->text2_pos = 0;
    matrix->text2_pixel_len = 0;
    for (int i=0; i<TEXT2_LEN; i++) {
        matrix->text2[i] = 0x0;
    }

    matrix->mask = 0xff;
    matrix->on = true;
    matrix->paused = false;
    matrix->scan_row = 0;

    return matrix;
}

void ultrathin_matrix_init(struct UltrathinLEDMatrix * const matrix,
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

void ultrathin_matrix_scan(struct UltrathinLEDMatrix *matrix) {
    if (!matrix->on) return;

    // Process each 8-pixel byte in the row
    uint8_t offset = row_offset(matrix, matrix->scan_row);

    // Process the row in reverse order
    for (int16_t x8=matrix->row_width-1; x8>=0; x8--) {
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

void ultrathin_matrix_clear(struct UltrathinLEDMatrix *matrix) {
    for (uint16_t i=0; i<(matrix->height*matrix->row_width); i++) {
        matrix->display_buffer[i] = 0x00;
    }
}

void ultrathin_matrix_set_pixel(struct UltrathinLEDMatrix *matrix, int16_t x, int16_t y) {
    bitWrite(matrix->display_buffer[buf_offset(matrix, x, y)], x % BYTE_WIDTH, 1);
}

void ultrathin_matrix_clear_pixel(struct UltrathinLEDMatrix *matrix, int16_t x, int16_t y) {
    bitWrite(matrix->display_buffer[buf_offset(matrix, x, y)], x % BYTE_WIDTH, 0);
}

void ultrathin_matrix_set_region(struct UltrathinLEDMatrix *matrix, char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    for (int16_t by=0; by<h; by++) {
        for (int16_t bx=0; bx<w; bx++) {
            int16_t _x = x+bx;
            int16_t _y = y+by;
            if (_x >= matrix->width || _x < 0) {
                continue;
            }

            uint8_t pixel8 = get_pixel8(buf, bx, by, w);
            uint8_t b = (bx % BYTE_WIDTH);

            if ((pixel8 << b) & 0x80) {
                ultrathin_matrix_set_pixel(matrix, _x, _y);
            }
            else {
                ultrathin_matrix_clear_pixel(matrix, _x, _y);
            }
        }
    }
}

void ultrathin_matrix_clear_region(struct UltrathinLEDMatrix *matrix, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    for (int16_t by=0; by<h; by++) {
        for (int16_t bx=0; bx<w; bx++) {
            int16_t X = x+bx;
            ultrathin_matrix_clear_pixel(matrix, X, y+by);
        }
    }
}

void ultrathin_matrix_start(struct UltrathinLEDMatrix *matrix) {
    matrix->paused = false;
}

void ultrathin_matrix_stop(struct UltrathinLEDMatrix *matrix) {
    matrix->paused = true;
}

void ultrathin_matrix_on(struct UltrathinLEDMatrix *matrix) {
    matrix->on = true;
}

void ultrathin_matrix_off(struct UltrathinLEDMatrix *matrix) {
    matrix->on = false;
    digitalWrite(matrix->oe, HIGH);
}

void ultrathin_matrix_reverse(struct UltrathinLEDMatrix *matrix) {
    matrix->mask = ~matrix->mask;
}

void ultrathin_matrix_write_char(struct UltrathinLEDMatrix *matrix, int16_t x, int16_t y, char c) {
    ultrathin_matrix_set_region(
            matrix,
            konker_bitfont_basic[(unsigned char)(c)],
            x, y,
            BYTE_WIDTH,CHARACTER_HEIGHT);
}

uint16_t ultrathin_matrix_set_text1(struct UltrathinLEDMatrix *matrix, const char *text) {
    strcpy(matrix->text1, text);
    matrix->text1_len = strlen(matrix->text1);
    matrix->text1_pixel_len = ultrathin_matrix_get_text1_pixel_len(matrix);
    return matrix->text1_pixel_len;
}

void ultrathin_matrix_set_text1_speed(struct UltrathinLEDMatrix *matrix, float speed) {
    matrix->text1_speed = speed;
}

void ultrathin_matrix_render_text1(struct UltrathinLEDMatrix *matrix, int16_t x_offset, int16_t y_offset) {
    uint16_t width_accum = 0;
    for (int8_t i=0; i<matrix->text1_len; i++) {
        int16_t _x = x_offset + width_accum;
        if (_x < matrix->width) {
            ultrathin_matrix_set_region(
                    matrix,
                    konker_bitfont_basic[(unsigned char)(matrix->text1[i])],
                    _x, y_offset,
                    BYTE_WIDTH, CHARACTER_HEIGHT);
        }
        width_accum +=
            (konker_bitfont_basic_metrics[(unsigned char)(matrix->text1[i])] + CHARACTER_SPACING);
    }
}

uint16_t ultrathin_matrix_get_text1_pixel_len(struct UltrathinLEDMatrix *matrix) {
    uint16_t ret = 0;
    for (int16_t i=0; i<matrix->text1_len; i++) {
        ret +=
            (konker_bitfont_basic_metrics[(unsigned char)(matrix->text1[i])] + CHARACTER_SPACING);
    }
    return ret;
}

uint16_t ultrathin_matrix_set_text2(struct UltrathinLEDMatrix *matrix, const char *text) {
    strcpy(matrix->text2, text);
    matrix->text2_len = strlen(matrix->text2);
    matrix->text2_pixel_len = ultrathin_matrix_get_text2_pixel_len(matrix);
    return matrix->text2_pixel_len;
}

void ultrathin_matrix_set_text2_speed(struct UltrathinLEDMatrix *matrix, float speed) {
    matrix->text2_speed = speed;
}

void ultrathin_matrix_render_text2(struct UltrathinLEDMatrix *matrix, int16_t x_offset, int16_t y_offset) {
    uint16_t width_accum = 0;
    for (int8_t i=0; i<matrix->text2_len; i++) {
        int16_t _x = x_offset + width_accum;
        if (_x < matrix->width) {
            ultrathin_matrix_set_region(
                    matrix,
                    konker_bitfont_basic[(unsigned char)(matrix->text2[i])],
                    _x, y_offset,
                    BYTE_WIDTH, CHARACTER_HEIGHT);
        }
        width_accum +=
            (konker_bitfont_basic_metrics[(unsigned char)(matrix->text2[i])] + CHARACTER_SPACING);
    }
}

uint16_t ultrathin_matrix_get_text2_pixel_len(struct UltrathinLEDMatrix *matrix) {
    uint16_t ret = 0;
    for (int16_t i=0; i<matrix->text2_len; i++) {
        ret +=
            (konker_bitfont_basic_metrics[(unsigned char)(matrix->text2[i])] + CHARACTER_SPACING);
    }
    return ret;
}

void ultrathin_matrix_tick(struct UltrathinLEDMatrix *matrix) {
    if (matrix->paused) return;

    // Animate and render text1
    matrix->text1_pos -= matrix->text1_speed;
    if (matrix->text1_pos < -1*matrix->text1_pixel_len) {
        matrix->text1_pos = matrix->width;
        ultrathin_matrix_clear_region(matrix, 0,TEXT1_Y, matrix->width,TEXT1_HEIGHT);
    }
    ultrathin_matrix_render_text1(matrix, round(matrix->text1_pos), TEXT1_Y);

    // Animate and render text2
    matrix->text2_pos -= matrix->text2_speed;
    if (matrix->text2_pos < -1*matrix->text2_pixel_len) {
        matrix->text2_pos = matrix->width;
        ultrathin_matrix_clear_region(matrix, 0,TEXT2_Y, matrix->width,TEXT2_HEIGHT);
    }
    ultrathin_matrix_render_text2(matrix, round(matrix->text2_pos), TEXT2_Y);

}

