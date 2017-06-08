#include "../../include/game/sapotache_interface.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include "game.h"
#include "game_client.h"

struct tile_s {
    enum card_id card;
    enum direction dir;
};


struct state_s {
    unsigned int id;
    int axe_broken;
    int lamp_broken;
    int cart_broken;
};


struct board_s{
    struct tile_s **tab;
    unsigned int width;
    unsigned int height;
    size_t n_objectives;
    struct position *objectives;
    struct position start;
    size_t n_holes;
    struct position *holes;
};


struct game_s {
    unsigned int id;
    enum p_type type; 
    unsigned int n_cards_total;
    size_t n_player_cards;
    enum card_id *player_cards;
    unsigned int n_players;
    struct board_s board;
    struct state_s *players_state; // players_state[id] = Player(id)'s tools state'
};



game_t my_game;


char const* get_player_name(){
    return "C3PO";
}


int initialize(unsigned int id,
               enum p_type type,
               unsigned int width,
               unsigned int height,
               struct position start,
               size_t n_objectives,
               struct position const objectives[],
               size_t n_holes,
               struct position const holes[],
               unsigned int n_cards_total,
               size_t n_player_cards,
               enum card_id const player_cards[],
               unsigned int n_players){

    assert(id < n_players);
    my_game = gameClient_new(width,height,n_objectives,objectives,start,n_holes,holes,n_cards_total,n_players,id,type,n_player_cards,player_cards);
    // init random seed
    srand(time(NULL));
    return 0;
}

struct move play(struct move const previous_moves[], size_t n_moves){
    struct move prev;
    size_t x;
    size_t y;
    //updating the board
    for( size_t i = 0; i < n_moves; i++ ){
        prev = previous_moves[i];
        x = prev.onto.x;
        y = prev.onto.y;
        switch( previous_moves[i].act ){
        case ADD_PATH_CARD:
            game_set_dir( my_game, x, y, prev.dir);
            game_set_card( my_game, x, y, prev.card);
            break;
        case PLAY_BOULDER_CARD:
            game_set_card(my_game, x, y, NO_CARD);
            break;
        case PLAY_BREAK_CARD:
            switch( prev.card ){
            case CARD_B_AXE:
                game_player_break_axe(my_game, prev.onplayer);
                break;
            case CARD_B_LAMP:
                game_player_break_lamp(my_game, prev.onplayer);
                break;
            case CARD_B_CART:
                game_player_break_cart(my_game, prev.onplayer);
                break;
            default:
                fprintf(stderr, "%d\n", prev.card);
            }
            break;
        case PLAY_REPAIR_CARD:
            switch( prev.card ){
            case CARD_R_AXE:
                game_player_repair_axe(my_game, prev.onplayer);
                break;
            case CARD_R_LAMP:
                game_player_repair_lamp(my_game, prev.onplayer);
                break;
            case CARD_R_CART:
                game_player_repair_cart(my_game, prev.onplayer);
                break;
            case CARD_R_ALL:
                game_player_repair_axe(my_game, prev.onplayer);
                game_player_repair_lamp(my_game, prev.onplayer);
                game_player_repair_cart(my_game, prev.onplayer);
                break;
            default:
                fprintf(stderr, "%d\n", prev.card);
            }
            break;
        case DISCARD:
        case FAILED:
            break;
        }
    }

    //playing the move
    struct move my_move;
    my_move.player = gameClient_get_id(my_game);
    int i_card = rand() % gameClient_get_n_player_cards(my_game);
    my_move.card = gameClient_get_ith_player_cards(my_game,i_card);
    gameClient_remove_card(my_game, i_card);

    size_t n_p = game_get_n_players(my_game);
    unsigned int wi = game_get_width(my_game);
    unsigned int he = game_get_height(my_game);
    if (my_move.card >= CARD_V_LINE && my_move.card <= CARD_R_TURN) {
        my_move.act = ADD_PATH_CARD;
        my_move.onto.x = rand() % wi;
        my_move.onto.y = rand() % he;
        my_move.dir = rand() % REVERSED;
    }else if (my_move.card >= CARD_B_AXE && my_move.card <= CARD_B_CART) {
        my_move.act = PLAY_BREAK_CARD;
        int target = rand() % n_p;
        my_move.onplayer = target;
    }else if (my_move.card >= CARD_R_AXE && my_move.card <= CARD_R_ALL) {
        my_move.act = PLAY_REPAIR_CARD;
        int target = rand() % n_p;
        my_move.onplayer = target;
    }else if (my_move.card == CARD_BOULDER) {
        my_move.act = PLAY_BOULDER_CARD;
        my_move.onto.x = rand() % wi;
        my_move.onto.y = rand() % he;
    }
    return my_move;
}

int draw_card(enum card_id card){
    if (card != NO_CARD)
        return gameClient_add_card(my_game, card);
    return 1;
}

int finalize(){
    game_free(my_game);
    return 0;
}
