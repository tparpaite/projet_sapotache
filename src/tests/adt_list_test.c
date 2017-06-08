#include <stdlib.h>
#include <stdio.h>
#include "adt_list.h"

static int *int_copy(int *x) {
    int *res = malloc(sizeof (int));
    *res = *x;
    return res;
}


static int int_free(int *x) {
    free(x);
    return 1;
}


static int int_print(int *x) {
    printf("%d", *x);
    return 1;
}


int main() {
    list_t l = list_create((copy_function_t) int_copy,
                           (free_function_t) int_free,
                           (print_function_t) int_print);

    for (int i = 0 ; i < 10 ; ++i) {
        list_add_head(l, (void *) &i);
    }

    list_print(l);
    list_shuffle(l);
    printf("\n");
    list_print(l);
    list_shuffle(l);
    printf("\n");
    list_print(l);

    list_t l_copy = list_copy(l);
    printf("l_copy : ");
    list_print(l_copy);
    printf("\n");

    printf("Shuffle l_copy : ");
    list_shuffle(l);
    list_print(l_copy);
    printf("\n");

    printf("l : ");
    list_print(l);
    printf("\n");

    return EXIT_SUCCESS;
}
    

