/**
 * \file lexer.h
 * \brief Contains the declaration of the functions used to parse a file
 * \author PARPAITE Thibault <br>
 * \author SARRABAYROUSE Alexis <br>
 * \date 06/03/2017
 */

#ifndef _PARSER_H
#define _PARSER_H

#include <stdio.h>
#include "adt_list.h"
#include "adt_coord.h"
#include "token.h"

struct memory_s;
typedef struct memory_s *memory_t;

extern int parse(FILE *f);

/* Used by server to create game from file */
extern memory_t parse_memory(FILE *f);
extern int memory_get_width(memory_t mem);
extern int memory_get_height(memory_t mem);
extern coord_t memory_get_start(memory_t mem);
extern unsigned int memory_get_n_cards(memory_t mem);
extern size_t memory_get_n_objectives(memory_t mem);
extern list_t memory_get_objectives(memory_t mem);
extern size_t memory_get_n_holes(memory_t mem);
extern list_t memory_get_holes(memory_t mem);
extern list_t memory_get_l_deck(memory_t mem);

#endif /* _PARSER_H */
