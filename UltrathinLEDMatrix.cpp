/**
 * UltrathinLEDMatrix
 *
 * Author: Konrad Markus <konker@luxvelocitas.com>
 *
 */

#include <Arduino.h>
#include "UltrathinLEDMatrix.h"
#include "konker_bitfont_basic.h"

#define row_offset(y) (m_row_width*y)
#define buf_offset(x, y) (row_offset(y)+x/8)
#define is_set(buf, x, y, w, h) ((buf[w/8*y+x/8] >> (7 - (x % 8))) & 1)


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

    m_x_offset = 0;
    m_y_offset = 0;
    for (int i=0; i<TEXT1_LEN; i++) {
        m_text1[i] = 0x0;
    }

    m_mask = 0xff;
    m_active = false;
    m_scan_row = 0;
}

void UltrathinLEDMatrix::begin(uint8_t *display_buffer, uint8_t width, uint8_t height) {
    m_width = width;
    m_height = height;
    m_display_buffer = display_buffer;
    m_row_width = (m_width / 8);

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

void UltrathinLEDMatrix::set_pixel(uint16_t x, uint16_t y) {
    bitWrite(m_display_buffer[buf_offset(x, y)], x % 8, 1);
}

void UltrathinLEDMatrix::set_pixel(uint16_t x, uint16_t y, uint8_t b) {
    bitWrite(m_display_buffer[buf_offset(x, y)], b, 1);
}

void UltrathinLEDMatrix::clear_pixel(uint16_t x, uint16_t y) {
    bitWrite(m_display_buffer[buf_offset(x, y)], x % 8, 0);
}

void UltrathinLEDMatrix::clear_pixel(uint16_t x, uint16_t y, uint8_t b) {
    bitWrite(m_display_buffer[buf_offset(x, y)], b, 0);
}

void UltrathinLEDMatrix::set_region(char *buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (int16_t by=0; by<h; by++) {
        for (int16_t bx=0; bx<w; bx++) {
            int16_t X = x+bx;
            if (X >= m_width || X < 0) {
                continue;
            }

            uint8_t pixel8 = buf[(by*w+bx)/8];
            uint8_t b = (bx % 8);
            uint8_t p = (pixel8 << b) & 0x80;

            if (p) {
                set_pixel(X, y+by);
            }
            else {
                clear_pixel(X, y+by);
            }
        }
    }
}

void UltrathinLEDMatrix::set_region_hflip(char *buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (int16_t by=0; by<h; by++) {
        if (x+by > m_height) {
            continue;
        }
        for (int16_t bx=0; bx<w; bx++) {
            if (x+bx > m_width) {
                continue;
            }
            uint8_t pixel8 = buf[(by*w+bx)/8];
            uint8_t b = (bx % 8);
            uint8_t p = (pixel8 >> b) & 0x01;

            if (p) {
                set_pixel(x+bx, y+by, b);
            }
            else {
                clear_pixel(x+bx, y+by, b);
            }
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

void UltrathinLEDMatrix::set_offset(uint16_t x_offset, uint16_t y_offset) {
    m_x_offset = x_offset;
    m_y_offset = y_offset;
}

void UltrathinLEDMatrix::render_text1(uint16_t x_offset) {
    for (int8_t i=0; i<strlen(m_text1); i++) {
        if (x_offset+6*i < m_width) {
            write_char(x_offset+6*i, 0, m_text1[i]);
        }
    }
}

void UltrathinLEDMatrix::set_text1(const char *text1) {
    strcpy(m_text1, text1);
}

void UltrathinLEDMatrix::write_char(uint16_t x, uint16_t y, char c) {
    set_region(konker_bitfont_basic[c], x,y, 8,6);
}

