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
 * konker_ultrathin_led_matrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with konker_ultrathin_led_matrix.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KONKER_ULTRATHIN_LED_SEGMENT_LIST_H__
#define __KONKER_ULTRATHIN_LED_SEGMENT_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "kulm_segment.h"


/**
 * A singly linked list of segments
 */
typedef struct kulm_segment_list
{
    kulm_segment * item;
    struct kulm_segment_list *next;

} kulm_segment_list;


kulm_segment_list * const kulm_segment_list_create(kulm_segment * const item);
void kulm_segment_list_destroy(kulm_segment_list * const head);
void kulm_segment_list_append(kulm_segment_list * const head, kulm_segment * const new_item);

uint16_t kulm_segment_list_get_length(kulm_segment_list * const head);
kulm_segment * const kulm_segment_list_get_nth(kulm_segment_list * const head, int16_t n);

#ifdef __cplusplus
}
#endif

#endif // __KONKER_ULTRATHIN_LED_SEGMENT_LIST_H__
