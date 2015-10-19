/**
 * UltrathinLED
 *
 * A library for driving the Seeedstudio Ultrathin red LED matrix
 *
 * Author: Konrad Markus <konker@luxvelocitas.com>
 */

#ifndef __ULTRATHIN_LED_MATRIX_H__
#define __ULTRATHIN_LED_MATRIX_H__

#include <stdint.h>

#define ULM_TEXT1_LEN 64
#define ULM_TEXT2_LEN 64

#define ULM_TEXT1_HEIGHT 8
#define ULM_TEXT2_HEIGHT 8

#define ULM_TEXT1_Y 0
#define ULM_TEXT2_Y 8


typedef struct ulm_matrix
{
    uint8_t  a, b, c, d, oe, r1, stb, clk;
    int16_t width;
    int16_t height;
    uint8_t  *display_buffer;
    uint8_t  mask;
    bool     on;
    bool     paused;

    char     text1[ULM_TEXT1_LEN];
    uint16_t text1_len;
    float    text1_speed;
    float    text1_pos;
    float    text1_pixel_len;

    char     text2[ULM_TEXT2_LEN];
    uint16_t text2_len;
    float    text2_speed;
    float    text2_pos;
    float    text2_pixel_len;

    uint16_t row_width;
    uint16_t scan_row;
} ulm_matrix;

ulm_matrix * const ulm_create(uint8_t *display_buffer, uint8_t width, uint8_t height);
void ulm_destroy(ulm_matrix * const matrix);
void ulm_init(ulm_matrix * const matrix,
              uint8_t a, uint8_t b, uint8_t c, uint8_t d,
              uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk);
void ulm_scan(ulm_matrix * const matrix);
void ulm_clear(ulm_matrix * const matrix);
inline void ulm_set_pixel(ulm_matrix * const matrix, int16_t x, int16_t y);
inline void ulm_clear_pixel(ulm_matrix * const matrix, int16_t x, int16_t y);
void ulm_set_region(ulm_matrix * const matrix, char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h);
void ulm_clear_region(ulm_matrix * const matrix, int16_t x, int16_t y, uint16_t w, uint16_t h);
void ulm_start(ulm_matrix * const matrix);
void ulm_stop(ulm_matrix * const matrix);
void ulm_on(ulm_matrix * const matrix);
void ulm_off(ulm_matrix * const matrix);
void ulm_reverse(ulm_matrix * const matrix);
void ulm_write_char(ulm_matrix * const matrix, int16_t x, int16_t y, char c);

uint16_t ulm_set_text1(ulm_matrix * const matrix, const char *text1);
void ulm_set_text1_speed(ulm_matrix * const matrix, float speed);
uint16_t ulm_get_text1_pixel_len(ulm_matrix * const matrix);
void ulm_render_text1(ulm_matrix * const matrix, int16_t x_offset, int16_t y_offset);

uint16_t ulm_set_text2(ulm_matrix * const matrix, const char *text2);
void ulm_set_text2_speed(ulm_matrix * const matrix, float speed);
uint16_t ulm_get_text2_pixel_len(ulm_matrix * const matrix);
void ulm_render_text2(ulm_matrix * const matrix, int16_t x_offset, int16_t y_offset);
void ulm_tick(ulm_matrix * const matrix);

#endif
