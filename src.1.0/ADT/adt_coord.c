/**
 * \file coord.c
 * \brief Contains the definitions of the functions used to create coord (point x, y)
 * \author PARPAITE Thibault <br>
 * \date 06/03/2017
 */

#include <stdlib.h>
#include <stdio.h>
#include "adt_coord.h"


/****************************************************
 * PRIVATE : TYPEDEF, STRUCTURES, STATICS FUNCTIONS *
 ****************************************************/

struct coord_s {
    int x;
    int y;
};


/********************
 * PUBLIC FUNCTIONS *
 ********************/

coord_t coord_create(int x, int y) {
    coord_t c = malloc(sizeof (struct coord_s));
    c->x = x;
    c->y = y;

    return c;
}


coord_t coord_copy(coord_t c) {
    coord_t tmp = malloc(sizeof (struct coord_s));
    tmp->x = c->x;
    tmp->y = c->y;

    return tmp;
}

        
int coord_free(coord_t c) {
    free(c);
    return 1;
}


int coord_getX(coord_t c) {
    return c->x;
}


int coord_getY(coord_t c) {
    return c->y;
}


int coord_setX(coord_t c, int x) {
    c->x = x;
    return 1;
}


int coord_setY(coord_t c, int y) {
    c->y = y;
    return 1;
}


int coord_print(coord_t c) {
    printf("(%d,%d)", c->x, c->y);
    return 1;
}
