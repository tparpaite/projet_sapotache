#include "game_server.h"
#include "adt_list.h"
struct player_s {
    int id;
    void *handler;
    client_play_t play;
    client_draw_card_t draw_card;
    enum p_type type;
    size_t n_player_cards;
    enum card_id *player_cards;
    int gold;
    int kicked;
};

struct server_extra_data_s {
    list_t deck;
    size_t n_moves;
    struct player_s *players_a;
    struct move *previous_moves;
};

int gameServer_extra_data_free(void *this){
    struct server_extra_data_s *extra_data = this;
    free(extra_data->players_a);
    free(extra_data->previous_moves);
    free(extra_data);
    return 0;
}

game_t gameServer_new(  unsigned int width,
                        unsigned int height,
                        size_t n_objectives,
                        struct position const objectives[],
                        struct position start,
                        size_t n_holes,
                        struct position const holes[],
                        unsigned int n_cards_total,
                        unsigned int n_players,

                        list_t const deck
                ){
    struct server_extra_data_s *extra_data = malloc(sizeof(*extra_data));
    extra_data->deck = list_copy(deck);
    // list_shuffle(extra_data->deck);
    extra_data->n_moves = 0;
    extra_data->players_a = malloc(sizeof(*extra_data->players_a) * n_players);
    for (size_t i = 0; i < n_players; i++) {
        extra_data->players_a[i].id = 1;
        extra_data->players_a[i].handler = NULL;
        extra_data->players_a[i].play = NULL;
        extra_data->players_a[i].draw_card = NULL;
        extra_data->players_a[i].type = HONEST;
        extra_data->players_a[i].n_player_cards = 0;
        extra_data->players_a[i].player_cards = NULL;
        extra_data->players_a[i].kicked = 0;
        extra_data->players_a[i].gold = 0;
    }

    extra_data->previous_moves = malloc(sizeof(*extra_data->previous_moves) * n_players);
    game_t this = game_new( width, height, n_objectives, objectives, start,
                            n_holes, holes, n_cards_total, n_players,
                            extra_data, gameServer_extra_data_free);
    return this;
}

int gameServer_player_init(
    game_t this,
    int id,
    void *handler,
    client_play_t fct_play,
    client_draw_card_t fct_draw_card,
    enum p_type type,
    size_t n_player_cards,
    enum card_id *player_cards
){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->players_a[id].id = id;
    extra_data->players_a[id].handler = handler;
    extra_data->players_a[id].play = fct_play;
    extra_data->players_a[id].draw_card = fct_draw_card;
    extra_data->players_a[id].type = type;
    extra_data->players_a[id].n_player_cards = n_player_cards;
    extra_data->players_a[id].player_cards = player_cards;
    return 0;

}

int gameServer_player_set_ith_previous_moves(game_t this, size_t i, struct move m){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->previous_moves[i] = m;
    return 0;
}
int gameServer_inc_n_moves(game_t this){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->n_moves++;
    return 0;
}
int gameServer_dec_n_moves(game_t this){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->n_moves--;
    return 0;
}
int gameServer_player_kick(game_t this, size_t player_id){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->players_a[player_id].kicked = 1;
    return 0;
}
int gameServer_player_is_kicked(game_t this, size_t i){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[i].kicked;
}


size_t gameServer_get_n_moves(game_t this){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->n_moves;
}
// player_t *gameServer_get_players_a(game_t this);
struct move gameServer_get_ith_previous_move(game_t this, size_t i){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->previous_moves[i];

}

enum card_id gameServer_pop_card(game_t this){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    enum card_id card;
    if (!list_isempty(extra_data->deck)) {
        list_begin(extra_data->deck);
        card = *(enum card_id *) list_getdata(extra_data->deck);
        list_remove_head(extra_data->deck);
    } else {
        card = NO_CARD;
    }

    return card;
}
int gameServer_deck_is_empty(game_t this){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return list_isempty(extra_data->deck);
}


void *gameServer_player_get_handler(game_t this, unsigned int id){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].handler;
}

struct move gameServer_player_play(game_t this, unsigned int id, struct move const previous_moves[], size_t n_moves){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].play(previous_moves, n_moves);
}
int gameServer_player_draw_card(game_t this, unsigned int id, enum card_id card){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].draw_card(card);
}

struct move *gameServer_get_previous_moves(game_t this){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->previous_moves;
}

enum p_type gameServer_player_get_type(game_t this, unsigned int id){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].type;
}
size_t gameServer_player_get_n_cards(game_t this, unsigned int id){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].n_player_cards;
}
enum card_id gameServer_player_get_ith_cards(game_t this, unsigned int id, size_t i){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].player_cards[i];
}
int gameServer_player_get_gold(game_t this, unsigned int id){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    return extra_data->players_a[id].gold;
}


enum p_type gameServer_player_set_type(game_t this, unsigned int id, enum p_type new_type){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->players_a[id].type = new_type;
    return 0;
}
size_t gameServer_player_set_n_cards(game_t this, unsigned int id, size_t new_n){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->players_a[id].n_player_cards = new_n;
    return 0;
}
enum card_id gameServer_player_set_ith_cards(game_t this, unsigned int id, size_t i, enum card_id new_card){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->players_a[id].player_cards[i] = new_card;
    return 0;
}
int gameServer_player_set_gold(game_t this, unsigned int id, int new_n){
    struct server_extra_data_s *extra_data = game_get_extra_data(this);
    extra_data->players_a[id].gold = new_n;
    return 0;
}
