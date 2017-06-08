#include "../../include/game/sapotache_interface.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>


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
    enum p_type type; // Type de nain
    unsigned int n_cards_total;
    size_t n_player_cards;
    enum card_id *player_cards;
    unsigned int n_players;
    struct board_s board;
    struct state_s *players_state; // players_state[id] = Player(id)'s tools state'
};


struct game_s my_game;

static int remove_card(size_t i_card);

char const* get_player_name(){
    return "C3PO";
}

// TODO : changer le void
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
    my_game.id = id;
    my_game.type = type;
    my_game.n_cards_total = n_cards_total;
    my_game.n_player_cards = n_player_cards;
    // TODO quelle taille pour le tableau player_cards? n_cards_total c'est pas mal
    my_game.player_cards = malloc(sizeof(*(my_game.player_cards)) * n_cards_total);
    if(my_game.player_cards == NULL){
        fprintf(stderr, "Error:%s:%d malloc Error\n",__FILE__ , __LINE__ );
        return 1;
    }
    for (size_t i = 0; i < n_player_cards; i++) {
        my_game.player_cards[i] = player_cards[i];
    }
    my_game.n_players = n_players;

    my_game.board.tab = malloc(sizeof(*(my_game.board.tab)) * width);
    if(my_game.board.tab == NULL){
        fprintf(stderr, "Error:%s:%d malloc Error\n",__FILE__ , __LINE__ );
        return 2;
    }
    for (size_t i = 0; i < width ; i++) {
        my_game.board.tab[i] = malloc(sizeof(*(my_game.board.tab[i])) * height);
        if(my_game.board.tab[i] == NULL){
            fprintf(stderr, "Error:%s:%d malloc Error(i = %lu)\n",__FILE__ , __LINE__, i );
            return 3;
        }
        for (size_t j = 0; j < height ; j++) {
            my_game.board.tab[i][j].card = NO_CARD;
            my_game.board.tab[i][j].dir  = NORMAL;
        }
    }


    my_game.board.width = width;
    my_game.board.height = height;
    my_game.board.n_objectives = n_objectives;
    my_game.board.objectives = malloc(sizeof(*(my_game.board.objectives)) * n_objectives);
    if(my_game.board.objectives == NULL){
        fprintf(stderr, "Error:%s:%d malloc Error\n",__FILE__ , __LINE__);
        return 4;
    }

    for (size_t i = 0; i < n_objectives; i++){
        my_game.board.objectives[i] = objectives[i];
    }

    my_game.board.start = start;
    my_game.board.n_holes = n_holes;
    // TODO : Possible d'avoir 0 hole??
    if (n_holes == 0) {
        my_game.board.holes = NULL;
    } else {
        my_game.board.holes = malloc(sizeof(*(my_game.board.holes)) * n_holes);
        if(my_game.board.holes == NULL){
            fprintf(stderr, "Error:%s:%d malloc Error\n",__FILE__ , __LINE__);
            return 5;
        }
    }

    for (size_t i = 0; i < n_holes; i++) {
        my_game.board.holes[i] = holes[i];
    }

    my_game.players_state = malloc(sizeof(*(my_game.players_state)) * n_players);
    if(my_game.players_state == NULL){
        fprintf(stderr, "Error:%s:%d malloc Error\n",__FILE__ , __LINE__);
        return 6;
    }

    for (size_t i = 0; i < n_players; i++) {
        my_game.players_state[i].id = i;
        my_game.players_state[i].axe_broken  = 0;
        my_game.players_state[i].lamp_broken = 0;
        my_game.players_state[i].cart_broken = 0;
    }
    // init random seed
    srand(time(NULL));
    return 0;
}

struct move play(struct move const previous_moves[], size_t n_moves){

    //update the board
    for( size_t i = 0; i < n_moves; i++ ){
        switch( previous_moves[i].act ){
        case ADD_PATH_CARD:
            my_game.board.tab[previous_moves[i].onto.x][previous_moves[i].onto.y].dir = previous_moves[i].dir;
            my_game.board.tab[previous_moves[i].onto.x][previous_moves[i].onto.y].card = previous_moves[i].card;
            break;
        case PLAY_BOULDER_CARD:
            my_game.board.tab[previous_moves[i].onto.x][previous_moves[i].onto.y].card = NO_CARD;
            break;
        case PLAY_BREAK_CARD:
            switch( previous_moves[i].card ){
            case CARD_B_AXE:
                my_game.players_state[previous_moves[i].onplayer].axe_broken = 1;
                break;
            case CARD_B_LAMP:
                my_game.players_state[previous_moves[i].onplayer].lamp_broken = 1;
                break;
            case CARD_B_CART:
                my_game.players_state[previous_moves[i].onplayer].cart_broken = 1;
                break;
            default:
                fprintf(stderr, "%d\n", previous_moves[i].card);
            }
            break;
        case PLAY_REPAIR_CARD:
            switch( previous_moves[i].card ){
            case CARD_R_AXE:
                my_game.players_state[previous_moves[i].onplayer].axe_broken = 0;
                break;
            case CARD_R_LAMP:
                my_game.players_state[previous_moves[i].onplayer].lamp_broken = 0;
                break;
            case CARD_R_CART:
                my_game.players_state[previous_moves[i].onplayer].cart_broken = 0;
                break;
            case CARD_R_ALL: //verifier le fonctionnement de cette fonction
                my_game.players_state[previous_moves[i].onplayer].axe_broken = 0;
                my_game.players_state[previous_moves[i].onplayer].lamp_broken = 0;
                my_game.players_state[previous_moves[i].onplayer].cart_broken = 0;
                break;
            default:
                fprintf(stderr, "%d\n", previous_moves[i].card);
            }
            break;
        case DISCARD:
        case FAILED:
            break;
        }
    }

    //play the move
    struct move my_move;
    my_move.player = my_game.id;

    // TODO : à quel momment on retire la carte de la main?
    // 			- Au momment de la jouer
    // 			- Au tour suivant avec previous_moves
    //
    // 		Pour l'instant on la supprime direct
    int i_card = rand() % my_game.n_player_cards;
    my_move.card = my_game.player_cards[i_card];
    remove_card(i_card);

    if (my_move.card >= CARD_V_LINE && my_move.card <= CARD_R_TURN) {
        my_move.act = ADD_PATH_CARD;
        my_move.onto.x = rand() % my_game.board.width;
        my_move.onto.y = rand() % my_game.board.height;
        my_move.dir = rand() % REVERSED;
    }else if (my_move.card >= CARD_B_AXE && my_move.card <= CARD_B_CART) {
        my_move.act = PLAY_BREAK_CARD;
        int target = rand() % my_game.n_players;
        my_move.onplayer = target;
    }else if (my_move.card >= CARD_R_AXE && my_move.card <= CARD_R_ALL) {
        my_move.act = PLAY_REPAIR_CARD;
        int target = rand() % my_game.n_players;
        my_move.onplayer = target;
    }else if (my_move.card == CARD_BOULDER) {
        my_move.act = PLAY_BOULDER_CARD;
        my_move.onto.x = rand() % my_game.board.width;
        my_move.onto.y = rand() % my_game.board.height;
    }
    return my_move;
}

int draw_card(enum card_id card){
    if (card != NO_CARD) {
        my_game.player_cards[my_game.n_player_cards] = card;
        my_game.n_player_cards++;
    }

    return 0;
}

int finalize(){
    free(my_game.player_cards);
    for (size_t i = 0 ; i < my_game.board.width ; ++i) 
        free(my_game.board.tab[i]);
      
    free(my_game.board.tab);
    free(my_game.board.objectives);
    free(my_game.board.holes);
    free(my_game.players_state);
    return 0;
}

/**
 * @brief Remove the i_card-th to the player hand
 * @param  i_card card index
 * @return        0 if the card was removed
 *                1 else
 */
static int remove_card(size_t i_card){ // TODO : On retourne jamais 1?
    assert(i_card < my_game.n_player_cards);
    my_game.n_player_cards--;
    for (size_t i = i_card; i < my_game.n_player_cards ; i++) {
        my_game.player_cards[i] = my_game.player_cards[i+1];
    }
    return 0;
}
