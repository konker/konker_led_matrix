/**
 * Konker's LED matrix library
 *
 * A library for driving the a LED matrix
 *
 * Copyright 2015, Konrad Markus <konker@luxvelocitas.com>
 *
 * This file is part of konker_led_matrix.
 *
 * konker_led_matrix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * konker_led_matrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with konker_led_matrix.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "klm_matrix.h"
#include "klm_segment.h"
#include "hexfont_iso-8859-15.h"

#define EXAMPLE_MATRIX_WIDTH 32
#define EXAMPLE_MATRIX_HEIGHT 16

#define EXAMPLE_A 0
#define EXAMPLE_B 2
#define EXAMPLE_C 3
#define EXAMPLE_D 1
#define EXAMPLE_R1 4
#define EXAMPLE_OE 21
#define EXAMPLE_STB 22
#define EXAMPLE_CLK 23

#ifdef KLM_NON_GPIO_MACHINE
#   define EXAMPLE_TEXT_SPEED1 2.20
#   define EXAMPLE_TEXT_SPEED2 4.20
#else
#   define EXAMPLE_TEXT_SPEED1 0.500
#   define EXAMPLE_TEXT_SPEED2 0.900
#endif


int main() {
    printf("Konker's LED Matrix library\n");

    if (!klm_mat_begin()) {
        fprintf(stderr, "Could not initialize matrix system. Aborting");
        exit(EXIT_FAILURE);
    }

    klm_config *example_config =
            klm_config_create(EXAMPLE_MATRIX_WIDTH, EXAMPLE_MATRIX_HEIGHT);
    klm_config_set_pin(example_config, 'a', EXAMPLE_A);
    klm_config_set_pin(example_config, 'b', EXAMPLE_B);
    klm_config_set_pin(example_config, 'c', EXAMPLE_C);
    klm_config_set_pin(example_config, 'd', EXAMPLE_D);
    klm_config_set_pin(example_config, 'o', EXAMPLE_OE);
    klm_config_set_pin(example_config, 'r', EXAMPLE_R1);
    klm_config_set_pin(example_config, 's', EXAMPLE_STB);
    klm_config_set_pin(example_config, 'x', EXAMPLE_CLK);

    // Create a matrix
    klm_matrix *example_matrix = klm_mat_create(stdout, example_config);

    // Initialize some font(s)
    hexfont * const example_font = hexfont_load_data(hexfont_iso_8859_15, 16);

    // Create a font list
    hexfont_list * const example_font_list = hexfont_list_create(NULL);

    // Add the font to the font list
    hexfont_list_append(example_font_list, example_font);

    // Create some segments
    klm_segment * const example_segment0 =
                            klm_seg_create(
                                        example_matrix,
                                        0,
                                        0,
                                        EXAMPLE_MATRIX_WIDTH/2,
                                        EXAMPLE_MATRIX_HEIGHT,
                                        0);

    klm_segment * const example_segment1 =
                            klm_seg_create(
                                        example_matrix,
                                        EXAMPLE_MATRIX_WIDTH/2 + 4,
                                        0,
                                        EXAMPLE_MATRIX_WIDTH/2 - 4,
                                        EXAMPLE_MATRIX_HEIGHT,
                                        0);

    // Create a segments list
    klm_segment_list * const example_segment_list = klm_segment_list_create(NULL);

    // Add the segments to the segment list
    klm_segment_list_append(example_segment_list, example_segment0);
    klm_segment_list_append(example_segment_list, example_segment1);

    // Initialize the matrix with the example segments and fonts
    klm_mat_init(example_matrix,
                 example_font_list,
                 example_segment_list);

    // Set some text in segment 0
    klm_seg_set_text(example_segment0, "HELLO");
    klm_seg_set_text_speed(example_segment0, -EXAMPLE_TEXT_SPEED1, 0);

    // Set some text in segment 1
    klm_seg_set_text(example_segment1, "WORLD!");
    klm_seg_set_text_speed(example_segment1, EXAMPLE_TEXT_SPEED2, 0);

    klm_seg_reverse(example_segment1);

    // Call the animation driver for a while
    int16_t j = 0;
    for (j=0; j<50000; j++) {
        // Call the display driver for one row
        klm_mat_scan(example_matrix);

#ifdef KLM_NON_GPIO_MACHINE
        if (example_matrix->scan_row == 0) {
            klm_mat_dump_buffer(example_matrix);
            sleep(1);
        }
#endif
    }

    // Clean up the matrix
    klm_mat_destroy(example_matrix);

    printf("Goodbye\n");
    return EXIT_SUCCESS;
}

