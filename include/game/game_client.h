#ifndef _GAME_CLIENT_H
#define _GAME_CLIENT_H
#include "game.h"

game_t gameClient_new(  unsigned int width,
                        unsigned int height,
                        size_t n_objectives,
                        struct position const objectives[],
                        struct position start,
                        size_t n_holes,
                        struct position const holes[],
                        unsigned int n_cards_total,
                        unsigned int n_players,
                        unsigned int id,
                    	enum p_type type,

                    	size_t n_player_cards,
                    	enum card_id const player_cards[]
                );

unsigned int gameClient_get_id(game_t this);
enum p_type gameClient_get_type(game_t this);
size_t gameClient_get_n_player_cards(game_t this);
enum card_id gameClient_get_ith_player_cards(game_t this, size_t i);

int gameClient_set_id(game_t this, unsigned int new_id);
int gameClient_set_type(game_t this, enum p_type);
int gameClient_set_n_player_cards(game_t this, size_t new_n);
int gameClient_set_ith_player_cards(game_t this, size_t i, enum card_id new_card);

/**
 * Remove the i_card-th to the player hand
 * @param  i_card card index
 * @return        0 if the card was removed
 *                1 else
 */
int gameClient_remove_card(game_t this, size_t i_card);

int gameClient_add_card(game_t this, size_t i_card);


#endif
