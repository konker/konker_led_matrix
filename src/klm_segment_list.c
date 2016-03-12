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

#include <stdlib.h>
#include <string.h>
#include "klm_segment.h"
#include "klm_segment_list.h"


klm_segment_list * const klm_segment_list_create(klm_segment * const item) {
    // Allocate memory for the head element
    klm_segment_list * const list = malloc(sizeof(klm_segment_list));

    list->item = item;
    list->next = NULL;

    return list;
}

void klm_segment_list_destroy(klm_segment_list * const head) {
    klm_segment_list *tmp;
    klm_segment_list *iter = head;

    while (iter->next) {
        tmp = iter;
        iter = iter->next;

        klm_seg_destroy(tmp->item);
        free(tmp);
    }
    klm_seg_destroy(iter->item);
    free(iter);
}

void klm_segment_list_append(klm_segment_list * const head, klm_segment * const new_item) {
    klm_segment_list *tail = head;
    while (tail->next) {
        tail = tail->next;
    }

    if (tail->item == NULL) {
        tail->item = new_item;
    }
    else {
        tail->next = klm_segment_list_create(new_item);
    }
}

uint16_t klm_segment_list_get_length(klm_segment_list * const head) {
    if (head == NULL) {
        return 0;
    }

    uint16_t ret = 0;
    klm_segment_list *iter;
    for (iter=head; iter!=NULL; iter=iter->next) {
        ret++;
    }
    return ret;
}

klm_segment * const klm_segment_list_get_nth(klm_segment_list * const head, int16_t n) {
    if (klm_segment_list_get_length(head) <= n) {
        return NULL;
    }

    int16_t i;
    klm_segment_list *iter = head;
    for (i=0; i<n; i++) {
        iter = iter->next;
    }
    return iter->item;
}

