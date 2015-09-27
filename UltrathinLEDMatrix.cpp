/**
 * UltrathinLEDMatrix
 *
 * Author: Konrad Markus <konker@luxvelocitas.com>
 *
 */

#include <Arduino.h>
#include "UltrathinLEDMatrix.h"
#include "konker_bitfont_basic.h"

#define BYTE_WIDTH 8
#define CHARACTER_HEIGHT 6
#define CHARACTER_SPACING 1

#define row_offset(y) (m_row_width*y)
#define buf_offset(x, y) (row_offset(y)+x/8)
#define is_set(buf, x, y, w, h) ((buf[w/8*y+x/8] >> (7 - (x % 8))) & 1)
#define buf_index(x, y, w) (y*w + x)/BYTE_WIDTH
#define get_pixel8(buf, x, y, w) buf[buf_index(x, y, w)];


UltrathinLEDMatrix::UltrathinLEDMatrix(
                    uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                    uint8_t oe, uint8_t r1, uint8_t stb, uint8_t clk)
{
    m_a = a;
    m_b = b;
    m_c = c;
    m_d = d;
    m_oe = oe;
    m_r1 = r1;
    m_stb = stb;
    m_clk = clk;

    for (int i=0; i<TEXT1_LEN; i++) {
        m_text1[i] = 0x0;
    }

    for (int i=0; i<TEXT2_LEN; i++) {
        m_text2[i] = 0x0;
    }

    m_mask = 0xff;
    m_active = false;
    m_scan_row = 0;
}

void UltrathinLEDMatrix::begin(uint8_t *display_buffer, uint8_t width, uint8_t height) {
    m_width = width;
    m_height = height;
    m_display_buffer = display_buffer;
    m_row_width = (m_width / BYTE_WIDTH);

    pinMode(m_a, OUTPUT);
    pinMode(m_b, OUTPUT);
    pinMode(m_c, OUTPUT);
    pinMode(m_d, OUTPUT);
    pinMode(m_oe, OUTPUT);
    pinMode(m_r1, OUTPUT);
    pinMode(m_clk, OUTPUT);
    pinMode(m_stb, OUTPUT);

    m_active = true;
}

void UltrathinLEDMatrix::scan() {
    if (!m_active) return;

    // Process each 8-pixel byte in the row
    uint8_t offset = row_offset(m_scan_row);

    // Process the row in reverse order
    for (int16_t x8=m_row_width-1; x8>=0; x8--) {
        uint8_t pixel8 = m_display_buffer[offset + x8];

        // Apply the mask
        pixel8 ^= m_mask;

        // Write each pixel in the byte, in reverse order
        shiftOut(m_r1, m_clk, MSBFIRST, pixel8);
    }

    // Disable display
    digitalWrite(m_oe, HIGH);

    // Display the rows in reverse order
    uint16_t display_row = (m_height - 1 - m_scan_row);

    // Select row
    digitalWrite(m_a, (display_row & 0x01));
    digitalWrite(m_b, (display_row & 0x02));
    digitalWrite(m_c, (display_row & 0x04));
    digitalWrite(m_d, (display_row & 0x08));

    // Latch data
    digitalWrite(m_stb, LOW);
    digitalWrite(m_stb, HIGH);
    digitalWrite(m_stb, LOW);

    // Enable display
    digitalWrite(m_oe, LOW);

    // Next row, wrap around at the bottom
    m_scan_row = (m_scan_row + 1) % m_height;
}

void UltrathinLEDMatrix::clear() {
    for (uint16_t i=0; i<(m_height*m_row_width); i++) {
        m_display_buffer[i] = 0x00;
    }
}

void UltrathinLEDMatrix::set_pixel(int16_t x, int16_t y) {
    bitWrite(m_display_buffer[buf_offset(x, y)], x % BYTE_WIDTH, 1);
}

void UltrathinLEDMatrix::set_pixel(int16_t x, int16_t y, uint8_t b) {
    bitWrite(m_display_buffer[buf_offset(x, y)], b, 1);
}

void UltrathinLEDMatrix::clear_pixel(int16_t x, int16_t y) {
    bitWrite(m_display_buffer[buf_offset(x, y)], x % BYTE_WIDTH, 0);
}

void UltrathinLEDMatrix::clear_pixel(int16_t x, int16_t y, uint8_t b) {
    bitWrite(m_display_buffer[buf_offset(x, y)], b, 0);
}

void UltrathinLEDMatrix::set_region(char *buf, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    for (int16_t by=0; by<h; by++) {
        for (int16_t bx=0; bx<w; bx++) {
            int16_t _x = x+bx;
            int16_t _y = y+by;
            if (_x >= m_width || _x < 0) {
                continue;
            }

            uint8_t pixel8 = get_pixel8(buf, bx, by, w);
            uint8_t b = (bx % BYTE_WIDTH);

            if ((pixel8 << b) & 0x80) {
                set_pixel(_x, _y);
            }
            else {
                clear_pixel(_x, _y);
            }
        }
    }
}

void UltrathinLEDMatrix::clear_region(int16_t x, int16_t y, uint16_t w, uint16_t h) {
    for (int16_t by=0; by<h; by++) {
        for (int16_t bx=0; bx<w; bx++) {
            int16_t X = x+bx;
            clear_pixel(X, y+by);
        }
    }
}

void UltrathinLEDMatrix::on() {
    m_active = true;
}

void UltrathinLEDMatrix::off() {
    m_active = false;
    digitalWrite(m_oe, HIGH);
}

void UltrathinLEDMatrix::reverse() {
    m_mask = ~m_mask;
}

void UltrathinLEDMatrix::write_char(int16_t x, int16_t y, char c) {
    set_region(
            konker_bitfont_basic[c],
            x, y,
            BYTE_WIDTH,CHARACTER_HEIGHT);
}

uint16_t UltrathinLEDMatrix::set_text1(const char *text) {
    strcpy(m_text1, text);
    m_text1_len = strlen(m_text1);
    return m_text1_len;
}

void UltrathinLEDMatrix::render_text1(int16_t x_offset, int16_t y_offset) {
    int16_t width_accum = 0;
    for (int8_t i=0; i<m_text1_len; i++) {
        int16_t _x = x_offset + width_accum;
        if (_x < m_width) {
            set_region(
                    konker_bitfont_basic[m_text1[i]],
                    _x, y_offset,
                    BYTE_WIDTH, CHARACTER_HEIGHT);
        }
        width_accum +=
            (konker_bitfont_basic_metrics[m_text1[i]] + CHARACTER_SPACING);
    }
}

uint16_t UltrathinLEDMatrix::get_text1_pixel_len() {
    uint16_t ret = 0;
    for (int16_t i=0; i<m_text1_len; i++) {
        ret +=
            (konker_bitfont_basic_metrics[m_text1[i]] + CHARACTER_SPACING);
    }
    return ret;
}

uint16_t UltrathinLEDMatrix::set_text2(const char *text) {
    strcpy(m_text2, text);
    m_text2_len = strlen(m_text2);
    return m_text2_len;
}

void UltrathinLEDMatrix::render_text2(int16_t x_offset, int16_t y_offset) {
    uint16_t width_accum = 0;
    for (int8_t i=0; i<m_text2_len; i++) {
        int16_t _x = x_offset + width_accum;
        if (_x < m_width) {
            set_region(
                    konker_bitfont_basic[m_text2[i]],
                    _x, y_offset,
                    BYTE_WIDTH, CHARACTER_HEIGHT);
        }
        width_accum +=
            (konker_bitfont_basic_metrics[m_text2[i]] + CHARACTER_SPACING);
    }
}

uint16_t UltrathinLEDMatrix::get_text2_pixel_len() {
    uint16_t ret = 0;
    for (int16_t i=0; i<m_text2_len; i++) {
        ret +=
            (konker_bitfont_basic_metrics[m_text2[i]] + CHARACTER_SPACING);
    }
    return ret;
}

