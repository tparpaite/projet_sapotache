#ifndef _GAME_SERVER_H
#define _GAME_SERVER_H
#include "game.h"
#include "../../include/ADT/adt_list.h"

typedef struct move (*client_play_t)(struct move const previous_moves[],
                                     size_t n_moves);
typedef int (*client_draw_card_t)(enum card_id card);


game_t gameServer_new(
    unsigned int width,
    unsigned int height,
    size_t n_objectives,
    struct position const objectives[],
    struct position start,
    size_t n_holes,
    struct position const holes[],
    unsigned int n_cards_total,
    unsigned int n_players,
    list_t const deck
);

int gameServer_player_init(
    game_t this,
    int id,
    void *handler,
    client_play_t fct_play,
    client_draw_card_t fct_draw_card,
    enum p_type type,
    size_t n_player_cards,
    enum card_id *player_cards
);


size_t gameServer_get_n_moves(game_t this);
// player_t *gameServer_get_players_a(game_t this);
struct move gameServer_get_ith_previous_move(game_t this, size_t i);

enum card_id gameServer_pop_card(game_t this);
int gameServer_deck_is_empty(game_t this);

struct move *gameServer_get_previous_moves(game_t this);



int gameServer_player_set_ith_previous_moves(game_t this, size_t i, struct move m);
int gameServer_inc_n_moves(game_t this);
int gameServer_player_kick(game_t this, size_t player_id);
int gameServer_dec_n_moves(game_t this);
int gameServer_player_is_kicked(game_t this, size_t i);




void *gameServer_player_get_handler(game_t this, unsigned int id);

struct move gameServer_player_play(game_t this, unsigned int id, struct move const previous_moves[], size_t n_moves);
int gameServer_player_draw_card(game_t this, unsigned int id, enum card_id card);

// int gameServer_player_set_play(game_t this, unsigned int id, client_play_t fct_play);
// int gameServer_player_set_draw_card(game_t this, unsigned int id, client_draw_card_t fct_draw_card);

enum p_type gameServer_player_get_type(game_t this, unsigned int id);
size_t gameServer_player_get_n_cards(game_t this, unsigned int id);
enum card_id gameServer_player_get_ith_cards(game_t this, unsigned int id, size_t i);
int gameServer_player_get_gold(game_t this, unsigned int id);


enum p_type gameServer_player_set_type(game_t this, unsigned int id, enum p_type new_type);
size_t gameServer_player_set_n_cards(game_t this, unsigned int id, size_t new_n);
enum card_id gameServer_player_set_ith_cards(game_t this, unsigned int id, size_t i, enum card_id new_card);
int gameServer_player_set_gold(game_t this, unsigned int id, int new_n);

#endif
