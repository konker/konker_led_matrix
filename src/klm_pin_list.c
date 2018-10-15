
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "klm_pin_list.h"


klm_pin_list * const klm_pin_list_create() {
    // Allocate memory for the pin list structure
    klm_pin_list * const list = malloc(sizeof(klm_pin_list));
    list->head = NULL;

    return list;
}

void klm_pin_list_destroy(klm_pin_list * const list) {
    // Traverse linked list and free each node
    __klm_pin_list_node_t * prev = list->head;
    __klm_pin_list_node_t * last = list->head;
    while (last != NULL) {
        prev = last;
        last = last->next;
        free(prev);
    }

    // Free the list structure itself
    free(list);
}

uint8_t klm_pin_list_get(klm_pin_list * const list, const char pin_name) {
    // Find the last node
    __klm_pin_list_node_t * last = list->head;
    while (last != NULL && last->key != pin_name) {
        last = last->next;
    }

    return last->value;
}

void klm_pin_list_put(klm_pin_list * const list, char pin_name, uint8_t pin_number) {
    // First node case
    if (list->head == NULL) {
        list->head = malloc(sizeof(__klm_pin_list_node_t));
        list->head->key = pin_name;
        list->head->value = pin_number;
        list->head->next = NULL;
        return;
    }

    // Find the last node
    __klm_pin_list_node_t * prev = list->head;
    __klm_pin_list_node_t * last = list->head;
    while (last != NULL) {
        prev = last;
        last = last->next;
    }

    // Create the new node and wire up the linked list
    last = malloc(sizeof(__klm_pin_list_node_t));
    last->key = pin_name;
    last->value = pin_number;
    last->next = NULL;
    prev->next = last;
    return;
}

