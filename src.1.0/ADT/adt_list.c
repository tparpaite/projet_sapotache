/**
 * \file list.c
 * \brief Contains the definitions of the functions used to create lists
 * \author PARPAITE Thibault <br>
 * \date 06/03/2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "adt_list.h"


/****************************************************
 * PRIVATE : TYPEDEF, STRUCTURES, STATICS FUNCTIONS *
 ****************************************************/

typedef struct node_s *node_t;

/**
 * \struct node_s
 * \brief a node used by the list
 *
 * Contains data and a pointer to the next node
 */
struct node_s {
    void *data;
    node_t next;
};

/**
 * \struct list_s
 * \brief a list
 *
 * That list can contain any type of data
 */
struct list_s {
    node_t head;
    node_t key;
    copy_function_t copy_function;
    free_function_t free_function;
    print_function_t print_function;
};


static void *list_create_node(list_t l, void *data, node_t next) {
    node_t n = malloc(sizeof (struct node_s));
    n->data = l->copy_function(data);
    n->next = next;
    return n;
}


static int list_free_node(list_t l, node_t n) {
    l->free_function(n->data);
    free(n);
    return 1;
}


/* http://stackoverflow.com/questions/6127503/shuffle-array-in-c */
/* arrange the N elements of ARRAY in random order.
 * Only effective if N is much smaller than RAND_MAX;
 * if this may not be the case, use a better random
 * number generator. */
static void shuffle_array(void *array, size_t n, size_t size) {
    char tmp[size];
    char *arr = array;
    size_t stride = size * sizeof(char);

    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; ++i) {
            size_t rnd = (size_t) rand();
            size_t j = i + rnd / (RAND_MAX / (n - i) + 1);

            memcpy(tmp, arr + j * stride, size);
            memcpy(arr + j * stride, arr + i * stride, size);
            memcpy(arr + i * stride, tmp, size);
        }
    }
}


/********************
 * PUBLIC FUNCTIONS *
 ********************/


/**
 * \fn list_t list_create()
 * \brief Create a new list
 * \brief Complexity : O(1)
 * \return board_size the board size
 */
list_t list_create(copy_function_t copy_f, free_function_t free_f, print_function_t print_f) {
    list_t l = malloc(sizeof (struct list_s));
    l->head = NULL;
    l->key = NULL;
    l->copy_function = copy_f;
    l->free_function = free_f;
    l->print_function = print_f;

    return l;
}


/**
 * \fn int list_isempty(list_t l)
 * \brief Return whether or not the list is empty
 * \brief Complexity : O(1)
 * \param l the list
 * \return int
 */
int list_isempty(list_t l) {
    return l->head == NULL;
}


/**
 * \fn int list_free(list_t l)
 * \brief Free a given list (and its data)
 * \brief Complexity: O(n) where n = the list size
 * \param l the list
 * \return void
 */
int list_free(list_t l) {
    list_begin(l);

    while (!list_isempty(l))
        list_remove_head(l);

    free(l);

    return 1;
}


/**
 * \fn void list_add_head(list_t l, void *data)
 * \brief Add an element at the head of the list
 * \brief Complexity: O(1)
 * \param l the list
 * \param data to add
 */
int list_add_head(list_t l, void *data) {
    node_t n = list_create_node(l, data, l->head);
    l->head = n;

    return 1;
}


/**
 * \fn void list_remove_head(list_t l)
 * \brief Remove the element at the head of the list
 * \brief Complexity: O(1)
 * \param l the list
 */
int list_remove_head(list_t l) {
    if (list_isempty(l)) {
        perror("list.c: at list_remove_head");
        perror("list is empty");
	return 0;
    }

    node_t tmp = l->head;
    l->head = tmp->next;
    list_free_node(l, tmp);
    l->key = l->head;

    return 1;
}


/**
 * \fn void list_add_tail(list_t l, void *data)
 * \brief Add an element at the tail of the list
 * \brief Complexity: O(n)
 * \param l the list
 * \param data to add
 */
int list_add_tail(list_t l, void *data) {
    if (list_isempty(l))
        return list_add_head(l, data);

    /* Searching the last node */
    list_begin(l);

    while (l->key->next != NULL)
        list_next(l);

    node_t n = list_create_node(l, data, NULL);
    l->key->next = n;

    return 1;
}


/**
 * \fn void list_remove_head(list_t l)
 * \brief Remove the element at the tail of the list
 * \brief Complexity: O(n)
 * \param l the list
 */
int list_remove_tail(list_t l) {
    if (list_isempty(l) || (l->head->next == NULL))
        return list_remove_head(l);

    /* Searching the before last node */
    list_begin(l);

    while (l->key->next->next != NULL)
        list_next(l);

    list_free_node(l, l->key->next);
    l->key->next = NULL;

    return 1;
}


size_t list_size(list_t l) {
    size_t size = 0;

    list_begin(l);
    while (!list_isend(l)) {
        size++;
        list_next(l);
    }

    return size;
}


/**
 * \fn void list_print(list_t l)
 * \brief Print the list thanks to print_function
 * \brief Complexity: O(n)
 * \param l the list
 */
int list_print(list_t l) {
    list_begin(l);
    while (!list_isend(l)) {
        l->print_function(list_getdata(l));
        printf(" ");
        list_next(l);
    }

    return 1;
}

int list_shuffle(list_t l) {
    int n = list_size(l);
    node_t *nodes_a = malloc(n * sizeof(node_t));

    /* Putting nodes into the tab */
    int i = 0;
    list_begin(l);

    while (!list_isend(l)) {
        nodes_a[i] = l->key;
        ++i;
        list_next(l);
    }

    /* Shuffling tab */
    shuffle_array(nodes_a, n, sizeof(node_t));

    /* Modify the list the way the tab was shuffled */
    l->head = nodes_a[0];

    for (i = 1 ; i < n ; ++i)
        nodes_a[i-1]->next = nodes_a[i];

    nodes_a[i-1]->next = NULL;

    return 1;
}


list_t list_copy(list_t l) {
    list_t copy = list_create(l->copy_function, l->free_function, l->print_function);

    list_begin(l);
    while (!list_isend(l)) {
        list_add_tail(copy, l->key->data);
        list_next(l);
    }

    return copy;
}


/**
 * \fn void list_begin(list_t l)
 * \brief Place the cursor on the first element
 * \brief Complexity: O(1)
 * \param l the list
 */
int list_begin(list_t l) {
    l->key = l->head;
    return 1;
}


/**
 * \fn int list_isempty(list_t l)
 * \brief Return whether or not the cursor is at the end
 * \brief Complexity: O(1)
 * \param l the list
 * \return int
 */
int list_isend(list_t l) {
    return list_getdata(l) == NULL;
}


/**
 * \fn void list_next(list_t l)
 * \brief Make the cursor go the next item
 * \brief Complexity: O(1)
 * \param l the list
 */
int list_next(list_t l) {
    if (l->key == NULL)
        return 0;

    l->key = l->key->next;
    return 1;
}


/**
 * \fn void *list_getdata(list_t l)
 * \brief Return the current value of current data
 * \brief Complexity: O(1)
 * \param l the list
 * \return data of the cursor's node
 */
void *list_getdata(list_t l) {
    if (l->key == NULL)
        return NULL;

    return l->copy_function(l->key->data);
}
