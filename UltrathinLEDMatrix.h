/**
 * UltrathinLED
 *
 * A library for driving the Seeedstudio Ultrathin LED matrix
 *
 * Author: Konrad Markus <konker@luxvelocitas.com>
 */

#ifndef __ULTRATHIN_LED_MATRIX_H__
#define __ULTRATHIN_LED_MATRIX_H__
#define TEXT1_LEN 64
#define TEXT2_LEN 64

#include <stdint.h>


class UltrathinLEDMatrix
{
public:
    UltrathinLEDMatrix(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                        uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk);

    void begin(uint8_t *display_buffer, uint8_t width, uint8_t height);
    void scan();
    void clear();
    inline void set_pixel(int16_t x, int16_t y);
    inline void set_pixel(int16_t x, int16_t y, uint8_t b);
    inline void clear_pixel(int16_t x, int16_t y);
    inline void clear_pixel(int16_t x, int16_t y, uint8_t b);
    void set_region(char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h);
    void clear_region(int16_t x, int16_t y, uint16_t w, uint16_t h);
    void on();
    void off();
    void reverse();
    void write_char(int16_t x, int16_t y, char c);

    uint16_t set_text1(const char *text1);
    uint16_t get_text1_pixel_len();
    void render_text1(int16_t x_offset, int16_t y_offset);

    uint16_t set_text2(const char *text2);
    uint16_t get_text2_pixel_len();
    void render_text2(int16_t x_offset, int16_t y_offset);

private:
    uint8_t  m_a, m_b, m_c, m_d, m_oe, m_r1, m_stb, m_clk;
    int16_t m_width;
    int16_t m_height;
    uint8_t  *m_display_buffer;
    uint8_t  m_mask;
    bool     m_active;

    char     m_text1[TEXT1_LEN];
    uint16_t m_text1_len;

    char     m_text2[TEXT2_LEN];
    uint16_t m_text2_len;

    uint16_t m_row_width;
    uint16_t m_scan_row;
};

#endif
