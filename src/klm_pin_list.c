
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "klm_pin_list.h"

#define KLM_PIN_LIST_NO_CHAR -1
#define KLM_PIN_LIST_BYTE_WIDTH 8


static uint8_t __klm_pin_list_hash_function(const char pin_name);

klm_pin_list * const klm_pin_list_create() {
    // Allocate memory for the pin list structure
    klm_pin_list * const list = malloc(sizeof(klm_pin_list));

    // Allocate memory for rep
    list->rep = calloc((int)pow(2, sizeof(char) * KLM_PIN_LIST_BYTE_WIDTH),
                       sizeof(__klm_pin_list_node_t));

//printf("klm_pin_list: %d, %d, %d\n", (int)sizeof(char) * KLM_PIN_LIST_BYTE_WIDTH, (int)pow(2, sizeof(char) * KLM_PIN_LIST_BYTE_WIDTH), (int)pow(2, 8));
    return list;
}

void klm_pin_list_destroy(klm_pin_list * const list) {
    uint16_t i;
    for (i=0; i<sizeof(char); i++) {
        __klm_pin_list_node_t * node = list->rep[i];

        if (node != NULL) {
            free(node);
        }
    }
    free(list->rep);
    free(list);
}

uint8_t klm_pin_list_get(klm_pin_list * const list, const char pin_name) {
    const uint16_t key = __klm_pin_list_hash_function(pin_name);
    __klm_pin_list_node_t const * node = list->rep[key];
    if (node == NULL) {
        return KLM_PIN_LIST_NO_CHAR;
    }

    return node->value;
}

void klm_pin_list_put(klm_pin_list * const list, char pin_name, uint8_t pin_number) {
    const char key = __klm_pin_list_hash_function(pin_name);
    if (list->rep[key] == NULL) {
        // Initialize a node to hold the item
        list->rep[key] = malloc(sizeof(__klm_pin_list_node_t));
        list->rep[key]->key   = key;
        list->rep[key]->next  = NULL;
    }

    // Set the node value
    list->rep[key]->value = pin_number;
}


// ----------------------------------------------------------------------------
// Static helpers
static uint8_t __klm_pin_list_hash_function(const char pin_name) {
    return (uint8_t)pin_name;
}


