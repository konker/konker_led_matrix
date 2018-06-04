/**
 */

#ifndef __KLM_PIN_FONT_H_
#define __KLM_PIN_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


// Linked list node for keeping glyphs in buckets when there are collisions
typedef struct __klm_pin_list_node_t {
    char key;
    uint8_t value;
    struct __klm_pin_list_node_t * next;

} __klm_pin_list_node_t;

// A list of code points and a way of looking up code points efficiently
typedef struct klm_pin_list {
    __klm_pin_list_node_t **rep;
    uint16_t length;

} klm_pin_list;

klm_pin_list * const klm_pin_list_create();
void klm_pin_list_destroy(klm_pin_list * const list);

void klm_pin_list_put(klm_pin_list * const list, const char pin_name, uint8_t pin_number);
uint8_t klm_pin_list_get(klm_pin_list * const list, const char pin_name);

#ifdef __cplusplus
}
#endif

#endif // __KLM_PIN_FONT_H_

