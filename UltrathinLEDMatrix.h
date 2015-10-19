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

#define TEXT1_LEN 64
#define TEXT2_LEN 64

#define TEXT1_HEIGHT 8
#define TEXT2_HEIGHT 8

#define TEXT1_Y 0
#define TEXT2_Y 8

// Why aren't these in wiringPi?
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define BYTE_WIDTH 8
#define CHARACTER_HEIGHT 6
#define CHARACTER_SPACING 1

#define row_offset(matrix, y) (matrix->row_width*y)
#define buf_offset(matrix, x, y) (row_offset(matrix, y)+x/8)
#define is_set(buf, x, y, w, h) ((buf[w/8*y+x/8] >> (7 - (x % 8))) & 1)
#define buf_index(x, y, w) (y*w + x)/BYTE_WIDTH
#define get_pixel8(buf, x, y, w) buf[buf_index(x, y, w)];

struct UltrathinLEDMatrix
{
    uint8_t  a, b, c, d, oe, r1, stb, clk;
    int16_t width;
    int16_t height;
    uint8_t  *display_buffer;
    uint8_t  mask;
    bool     on;
    bool     paused;

    char     text1[TEXT1_LEN];
    uint16_t text1_len;
    float    text1_speed;
    float    text1_pos;
    float    text1_pixel_len;

    char     text2[TEXT2_LEN];
    uint16_t text2_len;
    float    text2_speed;
    float    text2_pos;
    float    text2_pixel_len;

    uint16_t row_width;
    uint16_t scan_row;
};

struct UltrathinLEDMatrix * const ultrathin_matrix_create(uint8_t *display_buffer, uint8_t width, uint8_t height);
void ultrathin_matrix_init(struct UltrathinLEDMatrix * const matrix,
                           uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                           uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk);
void ultrathin_matrix_scan(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_clear(struct UltrathinLEDMatrix * const matrix);
inline void ultrathin_matrix_set_pixel(struct UltrathinLEDMatrix * const matrix, int16_t x, int16_t y);
inline void ultrathin_matrix_clear_pixel(struct UltrathinLEDMatrix * const matrix, int16_t x, int16_t y);
void ultrathin_matrix_set_region(struct UltrathinLEDMatrix * const matrix, char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h);
void ultrathin_matrix_clear_region(struct UltrathinLEDMatrix * const matrix, int16_t x, int16_t y, uint16_t w, uint16_t h);
void ultrathin_matrix_start(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_stop(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_on(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_off(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_reverse(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_write_char(struct UltrathinLEDMatrix * const matrix, int16_t x, int16_t y, char c);

uint16_t ultrathin_matrix_set_text1(struct UltrathinLEDMatrix * const matrix, const char *text1);
void ultrathin_matrix_set_text1_speed(struct UltrathinLEDMatrix * const matrix, float speed);
uint16_t ultrathin_matrix_get_text1_pixel_len(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_render_text1(struct UltrathinLEDMatrix * const matrix, int16_t x_offset, int16_t y_offset);

uint16_t ultrathin_matrix_set_text2(struct UltrathinLEDMatrix * const matrix, const char *text2);
void ultrathin_matrix_set_text2_speed(struct UltrathinLEDMatrix * const matrix, float speed);
uint16_t ultrathin_matrix_get_text2_pixel_len(struct UltrathinLEDMatrix * const matrix);
void ultrathin_matrix_render_text2(struct UltrathinLEDMatrix * const matrix, int16_t x_offset, int16_t y_offset);
void ultrathin_matrix_tick(struct UltrathinLEDMatrix * const matrix);

#endif
