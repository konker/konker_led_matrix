/**
 * Konker's LED matrix library
 *
 * A library for driving a red LED matrix
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

#ifndef __KONKER_LED_MATRIX_CONFIG_H__
#define __KONKER_LED_MATRIX_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include "klm_pin_list.h"


typedef struct {
    // Data structure for holding GPIO control pins
    klm_pin_list * pin_list;

    // Physical dimensions of the display
    uint16_t width;
    uint16_t height;

} klm_config;

klm_config * const klm_config_create(int16_t width, int16_t height);
void klm_config_destroy(klm_config * const config);

void klm_config_set_pin(klm_config * const config, char pin_name, uint8_t pin_number);
uint8_t klm_config_get_pin(klm_config * const config, char pin_name);

#ifdef __cplusplus
}
#endif

#endif // __KONKER_LED_MATRIX_CONFIG_H__
