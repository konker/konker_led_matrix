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
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>

#include "kulm_matrix.h"
#include "kulm_segment.h"
#include "konker_hexfont_basic.h"

#define EXAMPLE_MATRIX_WIDTH 64
#define EXAMPLE_MATRIX_HEIGHT 16
#define EXAMPLE_MATRIX_ROW_WIDTH (EXAMPLE_MATRIX_WIDTH / 8)


int main() {
    printf("Konker's Ultrathin LED Matrix library\n");

    // Initialize WirinPi if necessary
#ifndef ARDUINO
#  ifndef NON_GPIO_MACHINE
    if (wiringPiSetup()) {
        printf("ERROR Initializing WiringPi. Exiting.");
        return -1;
    }
#  endif
#endif
    uint8_t example_display_buffer[
                    EXAMPLE_MATRIX_HEIGHT *
                    EXAMPLE_MATRIX_ROW_WIDTH];

    // Create a matrix
    kulm_matrix *example_matrix =
                    kulm_mat_create(
                            example_display_buffer,
                            EXAMPLE_MATRIX_WIDTH,
                            EXAMPLE_MATRIX_HEIGHT,
                            0, 2, 3, 12, 13, 14, 21, 22);

    // Initialize some font(s)
    hexfont * const example_font = hexfont_load_data(konker_hexfont_basic, 16);

    kulm_mat_simple_init(example_matrix, example_font);
    kulm_mat_simple_set_text(example_matrix, "KONKER IS INVINCIBLE!!");
    kulm_mat_simple_set_text_speed(example_matrix, 1.0);

    // Call the animation driver for ten thousand frames
    int16_t j = 0;
    for (j=0; j<10000; j++) {
        // Update the animation
        kulm_mat_tick(example_matrix);

        // Call the display driver for one row
        kulm_mat_scan(example_matrix);

#ifdef NON_GPIO_MACHINE
        kulm_mat_dump_buffer(example_matrix);
#endif
    }

    // Clean up the matrix
    kulm_mat_destroy(example_matrix);

    printf("Goodbye\n");
    return EXIT_SUCCESS;
}

