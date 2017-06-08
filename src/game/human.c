#include "sapotache_interface.h"
#include "game.h"
#include "game_client.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

struct tile_s{
    enum card_id card;
    enum direction dir;
};

struct state_s{
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

/**
 * @brief Makes a character string out of a card
 * @param card, a card
 * @param dir, a direction
 * @return a character string
 */
static const char* enum_card_to_string(enum card_id card, enum direction dir);

/**
 * @brief Displays the board at its current state
 *
 */
static void human_print_tab();



char const* get_player_name(){
    return "Jean-Mi";
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
    gameClient_new(width,height, n_objectives, objectives, start, n_holes, holes, n_cards_total, n_players, id, type, n_player_cards, player_cards);

    // init random seed
    srand(time(NULL));
    return 0;
}

static void print_hand(){
    for (size_t i = 0; i < gameClient_get_n_player_cards(my_game); i++) {
        printf("%lu : %s\n", i, enum_card_to_string(gameClient_get_ith_player_cards(my_game,i), NORMAL));
    }
}

struct move play(struct move const previous_moves[], size_t n_moves){
    printf("n_move : %lu\n", n_moves); 
    struct move prev;
    size_t x;
    size_t y;
 //update the board
    for( size_t i = 0 ; i < n_moves ; i++ ){
        printf("%lu mississipi\n", i);
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

    //play the move
    struct move my_move;
    my_move.player = gameClient_get_id(my_game);

    print_hand();
    human_print_tab();

    // char buffer[255];
    printf("Quelle carte jouer?\n");
    size_t i_card = 0;
    scanf("%lu", &i_card);

my_move.card = gameClient_get_ith_player_cards(my_game,i_card);
    gameClient_remove_card(my_game, i_card);


    if (my_move.card >= CARD_V_LINE && my_move.card <= CARD_D_END) {
        printf("Où jouer la carte?\nx?\n");
        scanf("%u", &my_move.onto.x);
        printf("y?\n");
        scanf("%u", &my_move.onto.y);
        printf("direction?\n0: %s\n1: %s\n",
               enum_card_to_string(my_move.card, NORMAL),
               enum_card_to_string(my_move.card, REVERSED));

        scanf("%d",(int*) &my_move.dir);

        my_move.act = ADD_PATH_CARD;
    }else if (my_move.card >= CARD_B_AXE && my_move.card <= CARD_B_CART) {
        my_move.act = PLAY_BREAK_CARD;
        printf("Sur quel joueur?\n");
        int target;
        scanf("%d", &target);
        my_move.onplayer = target;
    }else if (my_move.card >= CARD_R_AXE && my_move.card <= CARD_R_ALL) {
        my_move.act = PLAY_REPAIR_CARD;
        printf("Sur quel joueur?\n");
        int target;
        scanf("%d", &target);
        my_move.onplayer = target;
    }else if (my_move.card == CARD_BOULDER) {
        my_move.act = PLAY_BOULDER_CARD;
        my_move.dir = NORMAL;
        printf("Où jouer la carte?\nx?\n");
        scanf("%u", &my_move.onto.x);
        printf("y?\n");
        scanf("%u", &my_move.onto.y);
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


static const char* enum_card_to_string(enum card_id card, enum direction dir){
    switch (card) {
    case NO_CARD:
        return "NO_CARD ( * )";
    case CARD_V_LINE:
        return "CARD_V_LINE ( | )";
    case CARD_H_LINE:
        return "CARD_H_LINE (---)";
    case CARD_V_CROSS:
        if (dir == REVERSED) {
            return "CARD_V_CROSS *reversed* ( |-)";
        }
        return "CARD_V_CROSS (-| )";
    case CARD_H_CROSS:
        if (dir == REVERSED) {
            return "CARD_H_CROSS *reversed* (-,-)";
        }
        return "CARD_H_CROSS (-'-)";
    case CARD_X_CROSS:
        return "CARD_X_CROSS (-|-)";
    case CARD_L_TURN:
        if (dir == REVERSED) {
            return "CARD_L_TURN *reversed* ( '-)";
        }
        return "CARD_L_TURN (-, )";
    case CARD_R_TURN:
        if (dir == REVERSED) {
            return "CARD_R_TURN *reversed* (-' )";
        }
        return "CARD_R_TURN ( ,-)";
    case CARD_D_END:
        return "CARD_D_END ( X )";
    case CARD_BOULDER:
        return "CARD_BOULDER ( B )";
    case CARD_B_AXE:
        return "CARD_B_AXE (B A)";
    case CARD_B_LAMP:
        return "CARD_B_LAMP (B L)";
    case CARD_B_CART:
        return "CARD_B_CART (B C)";
    case CARD_R_AXE:
        return "CARD_R_AXE (R A)";
    case CARD_R_LAMP:
        return "CARD_R_LAMP (R L)";
    case CARD_R_CART:
        return "CARD_R_CART (R C)";
    case CARD_R_ALL:
        return "CARD_R_ALL (RAL)";
    }
    return "oops";
}



/**
 * @brief Displays holes, start and end positions
 * @param x, a coordinate
 * @param y, a coordinate
 * @return 1 if the coordinates are ones of a start, an objective or a hole
 *         0 otherwise
 */
static int print_positions(size_t x, size_t y){
    struct position start = game_get_start(my_game);
    struct position obj;
    struct position hole;
    if (start.x == x && start.y == y) {
        printf(" > ");
        return 1;
    }
    for (size_t i = 0 ; i < game_get_n_objectives(my_game) ; i++) {
        obj = game_get_ith_objective(my_game,i);
        if (obj.x == x && obj.y == y) {
            printf(" $ ");
            return 1;
        }
    }
    for (size_t i = 0 ; i < game_get_n_holes(my_game) ; i++) {
        hole = game_get_ith_hole(my_game,i);
        if (hole.x == x && hole.y == y) {
            printf(" %% ");
            return 1;
        }
    }

    return 0;
}


/* Not cache friendly */
static void human_print_tab() {
    printf("\n");
    for (size_t j = game_get_height(my_game)-1 ; j+1 > 0 ; j--) { /* j+1 because impossible to compare with >= */
        printf("%lu| ", j);
        enum direction dir;
        for (size_t i = 0 ; i < game_get_width(my_game) ; i++) {
            dir = game_get_dir(my_game,i,j);
            if (!print_positions(i, j)) {
                switch (game_get_card(my_game,i,j)) {
                case NO_CARD:
                    printf(" * ");
                    break;
                case CARD_V_LINE:
                    printf(" | ");
                    break;
                case CARD_H_LINE:
                    printf("---");
                    break;
                case CARD_V_CROSS:
                    if (dir == REVERSED) {
                        printf(" |-");
                    }else {
                        printf("-| ");
                    }
                    break;
                case CARD_H_CROSS:
                    if (dir == REVERSED) {
                        printf("-,-");
                    }else {
                        printf("-'-");
                    }
                    break;
                case CARD_X_CROSS:
                    printf("-|-");
                    break;
                case CARD_L_TURN:
                    if (dir == REVERSED) {
                        printf(" '-");
                    }else {
                        printf("-, ");
                    }
                    break;
                case CARD_R_TURN:
                    if (dir == REVERSED) {
                        printf("-' ");
                    }else {
                        printf(" ,-");
                    }
                    break;
                case CARD_D_END:
                    printf(" X ");
                    break;
                case CARD_BOULDER:
                    printf(" B ");
                    break;
                case CARD_B_AXE:
                    printf("B A");
                    break;
                case CARD_B_LAMP:
                    printf("B L");
                    break;
                case CARD_B_CART:
                    printf("B C");
                    break;
                case CARD_R_AXE:
                    printf("R A");
                    break;
                case CARD_R_LAMP:
                    printf("R L");
                    break;
                case CARD_R_CART:
                    printf("R L");
                    break;
                case CARD_R_ALL:
                    printf("RAL");
                    break;
                }
            }
        }
        printf("\n");
    }
    printf("y|x");
    for (size_t i = 0; i < game_get_width(my_game); i++) {
        printf("%3lu", i);
    }
    printf("\n");
}
