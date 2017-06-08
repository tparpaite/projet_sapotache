#ifndef _GAME_H
#define _GAME_H
#include "../../include/game/sapotache_interface.h"

typedef struct game_s *game_t;

/**
 * Freed the extra_data
 * @param  game_free_fct [description]
 * @return               [description]
 */
typedef int (*game_free_fct)(void *extra_data);


// TODO rename to abstract_game_new
//  and the other abstract funcitons
game_t game_new(    unsigned int width,
                    unsigned int height,
                    size_t n_objectives,
                    struct position const objectives[],
                    struct position start,
                    size_t n_holes,
                    struct position const holes[],
                    unsigned int n_cards_total,
                    unsigned int n_players,
                    void *extra_data,
                    game_free_fct free_extra_data
                );


unsigned int game_get_width(game_t this);

unsigned int game_get_height(game_t this);

enum card_id game_get_card(game_t this, size_t x, size_t y);

int game_set_card(game_t this, size_t x, size_t y, enum card_id new_card);

enum direction game_get_dir(game_t this, size_t x, size_t y);

int game_set_dir(game_t this, size_t x, size_t y, enum direction new_dir);

size_t game_get_n_objectives(game_t this);

struct position game_get_ith_objective(game_t this, size_t i);

struct position game_get_start(game_t this);

size_t game_get_n_holes(game_t this);

struct position game_get_ith_hole(game_t this, size_t i);

size_t game_set_n_objectives(game_t this, size_t new_n);

int game_set_ith_objective(game_t this, size_t i, struct position new_objective);

int game_set_start(game_t this, struct position new_start);

int game_set_n_holes(game_t this, size_t new_n);

int game_set_ith_hole(game_t this, size_t i, struct position new_hole);

unsigned int game_get_n_cards_total(game_t this);

unsigned int game_get_n_players(game_t this);

int game_set_n_cards_total(game_t this, unsigned int new_n);

int game_set_n_players(game_t this, unsigned int new_n);

void * game_get_extra_data(game_t this);

int game_free(game_t this);


int game_player_axe_is_broken(game_t this, unsigned int id);
int game_player_lamp_is_broken(game_t this, unsigned int id);
int game_player_cart_is_broken(game_t this, unsigned int id);

int game_player_break_axe(game_t this, unsigned int id);
int game_player_repair_axe(game_t this, unsigned int id);
int game_player_break_lamp(game_t this, unsigned int id);
int game_player_repair_lamp(game_t this, unsigned int id);
int game_player_break_cart(game_t this, unsigned int id);
int game_player_repair_cart(game_t this, unsigned int id);


int game_print_board(game_t this);

#endif
