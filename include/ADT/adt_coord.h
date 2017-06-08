/**
 * \file list.h
 * \brief Contains the declaration of the functions used to create coord (point : x, y)
 * \author PARPAITE Thibault <br>
 * \date 06/03/2017
 */

#ifndef _COORD_H
#define _COORD_H

typedef struct coord_s *coord_t;

extern coord_t coord_create(int x, int y);
extern coord_t coord_copy(coord_t c);
extern int coord_free(coord_t c);
extern int coord_getX(coord_t c);
extern int coord_getY(coord_t c);
extern int coord_setX(coord_t c, int x);
extern int coord_setY(coord_t c, int y);
extern int coord_print(coord_t c);

#endif /* _COORD_H */
