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

#include "klm_config.h"


klm_config * const klm_config_create(int16_t width, int16_t height) {
    // Allocate memory for the config structure
    klm_config * const config = malloc(sizeof(klm_config));

    config->pin_list = klm_pin_list_create();
    config->width = width;
    config->height = height;

    return config;
}

void klm_config_destroy(klm_config * const config) {
    klm_pin_list_destroy(config->pin_list);
    free(config);
}

void klm_config_set_pin(klm_config * const config, char pin_name, uint8_t pin_number) {
    klm_pin_list_put(config->pin_list, pin_name, pin_number);
}

uint8_t klm_config_get_pin(klm_config * const config, char pin_name) {
    return klm_pin_list_get(config->pin_list, pin_name);
}

