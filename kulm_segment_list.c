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
#include <string.h>
#include "kulm_segment.h"
#include "kulm_segment_list.h"


kulm_segment_list * const kulm_segment_list_create(kulm_segment * const item) {
    // Allocate memory for the head element
    kulm_segment_list * const list = malloc(sizeof(kulm_segment_list));

    list->item = item;
    list->next = NULL;

    return list;
}

void kulm_segment_list_destroy(kulm_segment_list * const head) {
    kulm_segment_list *tmp;
    kulm_segment_list *iter = head;

    while (iter->next) {
        tmp = iter;
        iter = iter->next;

        kulm_seg_destroy(tmp->item);
        free(tmp);
    }
    kulm_seg_destroy(iter->item);
    free(iter);
}

void kulm_segment_list_append(kulm_segment_list * const head, kulm_segment * const new_item) {
    kulm_segment_list *tail = head;
    kulm_segment_list *iter = head;
    while (iter->next) {
        tail = iter;
        iter = iter->next;
    }

    if (tail->item == NULL) {
        tail->item = new_item;
    }
    else {
        tail->next = kulm_segment_list_create(new_item);
    }
}

uint16_t kulm_segment_list_get_length(kulm_segment_list * const head) {
    uint16_t ret = 0;
    kulm_segment_list *iter = head;
    while (iter->next) {
        ret++;
        iter = iter->next;
    }
    return ret;
}

kulm_segment * const kulm_segment_list_get_nth(kulm_segment_list * const head, int16_t n) {
    if (kulm_segment_list_get_length(head) < n) {
        return NULL;
    }

    int16_t i;
    kulm_segment_list *iter = head;
    for (i=0; i<n; i++) {
        iter = iter->next;
    }
    return iter->item;
}

