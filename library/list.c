#include "list.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#define GROWTH_FACTOR 2

struct list {
    size_t max_size;
    size_t curr_size;
    free_func_t freer;
    void **data;
};

list_t *list_init(size_t initial_size, free_func_t freer) {
    if (initial_size == 0){
        initial_size++; 
    }
    void **data = malloc(initial_size * sizeof(void *));
    assert(data != NULL);
    list_t *res = malloc(sizeof(list_t));
    assert(res != NULL);
    res->max_size = initial_size;
    res->curr_size = 0;
    res->freer = freer;
    res->data = data;
    return res;
}

list_t *simple_list_init(size_t initial_size) {
    list_t *res = list_init(initial_size, NULL);
    return res;
}

// Helper function to free an item
void free_item(void *item, free_func_t freer) {
    if (freer != NULL) {
    
        freer(item);
    }  else {
        free(item); 
    }
}

void list_free(list_t *list) {
    for (size_t i = 0; i < list->curr_size; i++) {
        free_item(list_get(list, i), list->freer);

    }
    free(list->data);
    free(list);
}

size_t list_size(list_t *list) {
    return list->curr_size;
}

// Checks that the index is valid before retrieving said item
void *list_get(list_t *list, size_t index) {
    assert((index >= 0) && (index < list->curr_size));
    return list->data[index];
}

// Checks that list has space and value is non-NULL before adding item
void list_add(list_t *list, void *value) {
    assert(value != NULL);
    // Automatically resize if we run out of space
    assert(list->curr_size <= list->max_size);
    if (list->curr_size == list->max_size) {
        list->data =
            realloc(list->data, sizeof(void *) * list->curr_size * GROWTH_FACTOR);
        list->max_size *= 2;
    }
    list->data[list->curr_size] = value;
    list->curr_size++;
}

// Checks that list is non-empty before removing item at given index
// Does not free anything
void *list_remove(list_t *list, size_t index) {
    assert(list->curr_size > 0);
    assert(index >= 0);
    assert(index < list->curr_size);

    void *removed_val = list_get(list, index);

    // Shift all the other list contents down,
    // if index is last element this won't get called
    for (size_t i = index + 1; i < list->curr_size; i++) {
        list->data[i - 1] = list->data[i];
    }
    list->curr_size--;
    return removed_val;
}