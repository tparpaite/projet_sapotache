#include "../../include/game/game.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>



struct client_extra_data_s {
	unsigned int id;
	enum p_type type; 
	size_t n_player_cards;
	enum card_id *player_cards;
};

int gameClient_extra_data_free(void *this){
    struct client_extra_data_s *extra_data = this;
    free(extra_data->player_cards);
    free(extra_data);
    return 0;
}

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
                ){
    struct client_extra_data_s *extra_data = malloc(sizeof(*extra_data));
	extra_data->id = id;
	extra_data->type = type;
	extra_data->n_player_cards = n_player_cards;
	extra_data->player_cards = malloc(sizeof(*extra_data->player_cards) * n_player_cards);
    for (size_t i = 0; i < n_player_cards; i++) {
        extra_data->player_cards[i] = player_cards[i];
    }
    game_t this = game_new(	width, height, n_objectives, objectives, start,
		 					n_holes, holes, n_cards_total, n_players,
							extra_data, gameClient_extra_data_free);
    return this;
}

unsigned int gameClient_get_id(game_t this){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->id;
}
enum p_type gameClient_get_type(game_t this){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->type;
}
size_t gameClient_get_n_player_cards(game_t this){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->n_player_cards;
}
enum card_id gameClient_get_ith_player_cards(game_t this, size_t i){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->player_cards[i];
}

int gameClient_set_id(game_t this, unsigned int new_id){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->id = new_id;
    return 0;
}
int gameClient_set_type(game_t this, enum p_type new_type){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->type = new_type;
    return 0;
}
int gameClient_set_n_player_cards(game_t this, size_t new_n){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->n_player_cards = new_n;
    return 0;
}
int gameClient_set_ith_player_cards(game_t this, size_t i, enum card_id new_card){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->player_cards[i] = new_card;
    return 0;
}


int gameClient_remove_card(game_t this, size_t i_card){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->n_player_cards--;
    for (size_t i = i_card; i < extra_data->n_player_cards ; i++) {
        extra_data->player_cards[i] = extra_data->player_cards[i+1];
    }
    return 0;
}

int gameClient_add_card(game_t this, enum card_id new_card){
    struct client_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->player_cards[extra_data->n_player_cards] = new_card;
    extra_data->n_player_cards++;
    return 0;
}
