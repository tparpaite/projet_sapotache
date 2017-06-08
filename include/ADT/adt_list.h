/**
 * \file adt_list.h
 * \brief Contains the declaration of the functions used to create lists
 * \author PARPAITE Thibault <br>
 * \date 06/03/2017
 */

#ifndef _LIST_H
#define _LIST_H

#include <stdlib.h>

typedef struct list_s *list_t;
typedef void *(*copy_function_t)(void *data);
typedef int   (*free_function_t)(void *data);
typedef int   (*print_function_t)(void *data);

/* List functions */
extern list_t list_create(copy_function_t copy_f, free_function_t free_f, print_function_t print_f);
extern int list_isempty(list_t l);
extern int list_free(list_t l);
extern int list_add_head(list_t l, void *data);
extern int list_remove_head(list_t l);
extern int list_add_tail(list_t l, void *data);
extern int list_remove_tail(list_t l);
extern size_t list_size(list_t l);
extern int list_print(list_t l);
extern list_t list_copy(list_t l);
extern int list_shuffle(list_t l);

/* Cursor functions */
extern int list_begin(list_t l);
extern int list_isend(list_t l);
extern int list_next(list_t l);
extern void *list_getdata(list_t l);


#endif /* _LIST_H */
