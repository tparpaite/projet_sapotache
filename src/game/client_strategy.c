#include "sapotache_interface.h"
#include "game.h"
#include "game_client.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdio.h>

/*****************STRATEGIES********************/
/*1) Sapoteur
  2) Normal

1) Objectif: Empecher les nains normaux d'accéder au trésor

stratégies de base:
- repérer les nains normaux et casser leurs outils
  repérer les sapoteurs et réparer leurs outils si ils sont cassés
- mettre des chemins déviants des trésors
- si une seule des trois sorties correspond au trésor, si il y a
existance de la carte permettant de retourner une des sorties,
pousser un chemi vers une sortie qui n'est pas le trésor.
- aller systematiquement en direction opposée au trésor


2) Objectif: Accéder à un trésor

stratégies de base:
-   repérer les nains sapoteurs et casser leurs outils
  repérer les nains normaux et réparer leurs outils s'ils sont cassés.
- mettre des chemins menant au trésor
*/




/**********************STRUCTURES****************************/

struct tile_s{
    enum card_id card;
    enum direction dir;
    int N;
    int S;
    int W;
    int E;
};

struct state_s{
    unsigned int id;
    int axe_broken;
    int lamp_broken;
    int cart_broken;
    int confident_lvl;

};

#define CONFIDENT 2
#define UNCONFIDENT -2

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

/***********************HEADERS*********************************/

struct move copy_move_afromb( struct move a, struct move b );

static int ia_update_cardinalpoints(struct tile_s *t);

//static int ia_check_cardinalpoints(struct move m);



static struct move strategic_move( struct move const previous_moves[], size_t n_moves );
static int strategy_honest(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int strategy_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves );


static int  play_break_card(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int  play_repair_card(struct move my_move, struct move const previous_moves[], size_t n_moves);
static int play_path_card_honest(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int play_path_card_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves);
static int play_boulder_card(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int play_discard_honest(struct move my_move,  struct move const previous_moves[], size_t n_moves);
static int play_discard_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves );


/***********************UTILITARIAN FUNCTIONS*********************/

/**
 * @brief Creates a move a by copying a move b
 * @param a, a move
 * @param b, a move
 * @return a
 */


struct move copy_move_afromb( struct move a, struct move b ){
    a.player = b.player;
    a.onto.x = b.onto.x;
    a.onto.y = b.onto.y;
    a.dir = b.dir;
    a.onplayer = b.onplayer;
    a.card = b.card;
    a.act = b.act;
    return a;
}


/************FUNCTION TO INITIALIZE THE GAME***********************/


char const* get_player_name(){
    return "Ian-Arnaud";
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

/**********FUNCTIONS TO PREVENT UNAVAILABLE PLACEMENTS********/

static int ia_update_cardinalpoints(struct tile_s *t) {
    switch (t->card){
    case NO_CARD:
        t->N = 1;
        t->S = 1;
        t->W = 1;
        t->E = 1;
        break;
    case CARD_V_LINE:
        t->N = 1;
        t->S = 1;
        t->W = 0;
        t->E = 0;
        break;
    case CARD_H_LINE:
        t->N = 0;
        t->S = 0;
        t->W = 1;
        t->E = 1;
        break;
    case CARD_V_CROSS:
        if(t->dir == NORMAL){
            t->N = 1;
            t->S = 1;
            t->W = 1;
            t->E = 0;
            break;
        }
        else{
            t->N = 1;
            t->S = 1;
            t->W = 0;
            t->E = 1;
            break;
        }
    case CARD_H_CROSS:
        if(t->dir == NORMAL){
            t->N = 1;
            t->S = 0;
            t->W = 1;
            t->E = 1;
            break;
        }
        else{
            t->N = 0;
            t->S = 1;
            t->W = 1;
            t->E = 1;
            break;
        }
    case CARD_L_TURN:
        if(t->dir == NORMAL){
            t->N = 0;
            t->S = 1;
            t->W = 1;
            t->E = 0;
            break;
        }
        else{
            t->N = 1;
            t->S = 0;
            t->W = 0;
            t->E = 1;
            break;
        }
    case CARD_R_TURN:
        if(t->dir == NORMAL){
            t->N = 0;
            t->S = 1;
            t->W = 0;
            t->E = 1;
            break;
        }
        else{
            t->N = 1;
            t->S = 0;
            t->W = 1;
            t->E = 0;
            break;
        }
    case CARD_X_CROSS:
        t->N = 1;
        t->S = 1;
        t->W = 1;
        t->E = 1;
        break;
    case CARD_D_END:
        t->N = 0;
        t->S = 0;
        t->W = 0;
        t->E = 0;
        break;
    default:
        return 1;
    }


    return 0;
}


/*Returns 0 if the move can be executed: the given card can be placed
  at the given set of the board.
  Returns 1 otherwise. */
static int ia_check_cardinalpoints(struct move m) {
    /* storing the cards situated below, beside
       and above the one we would like to play */
    struct tile_s above;
    struct tile_s below;
    struct tile_s left;
    struct tile_s right;
    int x = m.onto.x;
    int y = m.onto.y;
    int h = game_get_height(my_game);
    int l = game_get_width(my_game);
    if( x > 0 )
        above = (my_game->board.tab[x -1][y]);
    else
        above.card = NO_CARD;
    if( x < h - 1 )
        below = (my_game->board.tab[x +1][y]);
    else
        below.card = NO_CARD;
    if( y > 0 )
        left = (my_game->board.tab[x][y - 1]);
    else
        left.card = NO_CARD;
    if( y < l - 1 )
        right = (my_game->board.tab[x][y + 1]);
    else
        right.card = NO_CARD;

    ia_update_cardinalpoints( &above );
    ia_update_cardinalpoints( &below );
    ia_update_cardinalpoints( &right );
    ia_update_cardinalpoints( &left );

    switch (m.card){
    case CARD_V_LINE:
        if( above.S == 1 && below.N ==1 )
            return 0;
        break;
    case CARD_H_LINE:
        if( left.E == 1 && right.W ==1 )
            return 0;
        break;
    case CARD_V_CROSS:
        if( m.dir == NORMAL && below.N ==1 && above.S ==1
            && left.E == 1 )
            return 0;
        if( m.dir == REVERSED && below.N ==1 && above.S ==1
            && right.W == 1 )
            return 0;
        break;
    case CARD_H_CROSS:
        if( m.dir == NORMAL && above.S ==1 && right.W ==1 &&
            left.E == 1 )
            return 0;
        if( m.dir == REVERSED && below.N == 1 && right.W ==1 &&
            left.E == 1 )
            return 0;
        break;
    case CARD_L_TURN:
        if( m.dir == NORMAL && below.N == 1 && left.E == 1 )
            return 0;
        if( m.dir == REVERSED && above.S == 1 && right.W == 1 )
            return 0;
        break;
    case CARD_R_TURN:
        if( m.dir == NORMAL && below.N == 1 && right.W == 1 )
            return 0;
        if( m.dir == REVERSED && above.S == 1 && left.E == 1 )
            return 0;
        break;
    case CARD_X_CROSS:
        if( below.N ==1 && above.S ==1 && right.W == 1 && left.E == 1 )
            return 0;
        break;
    default:
        return 1;
    }
    return 1;
    }



/******************STRATEGY***************************/



/**
 * @brief returns the strategic move that the player should play acording to the actual situation, depending on the type of the player.
 * @param previous_moves[] an array of moves
 * @param n_moves, the size of the array previous_moves
 * @return my_move
 */


static struct move strategic_move(struct move const previous_moves[], size_t n_moves ){
    struct move my_move;
    if(my_game->type == HONEST )
        strategy_honest( my_move, previous_moves, n_moves );
    if(my_game->type == SAPOTEUR )
        strategy_sapoteur( my_move, previous_moves, n_moves );
    return my_move;
}

/**
 * @brief returns the strategic move that the honest player should play
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int strategy_honest(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    my_move.player = my_game->id;
    play_path_card_honest( my_move, previous_moves, n_moves);
    if( ia_check_cardinalpoints(my_move)){
        play_repair_card( my_move, previous_moves, n_moves);
        if( ia_check_cardinalpoints(my_move)){
            play_break_card(my_move, previous_moves, n_moves);
            if( ia_check_cardinalpoints(my_move)){
                play_discard_honest(my_move, previous_moves, n_moves);
            }
        }
    }
    return 0;
}

/**
 * @brief returns the strategic move that the sapoteur should play
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int strategy_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    my_move.player = my_game->id;
    play_boulder_card( my_move, previous_moves, n_moves);
    if( ia_check_cardinalpoints(my_move)){
        play_break_card(my_move, previous_moves, n_moves);
        if( ia_check_cardinalpoints(my_move)){
            play_path_card_sapoteur( my_move, previous_moves, n_moves);
            if( ia_check_cardinalpoints(my_move)){
                play_discard_sapoteur(my_move, previous_moves, n_moves);
                if( ia_check_cardinalpoints(my_move)){
                    play_repair_card( my_move, previous_moves, n_moves);
                }
            }
        }
    }
    return 0;
}


/**
 * @brief plays the strategy to repair a tool
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int  play_repair_card(struct move my_move, struct move const previous_moves[], size_t n_moves ){

    for(unsigned int i=0; i < my_game->n_player_cards; i++){ //the player look at his n_player_cards hand
        if (my_game->player_cards[i] >= CARD_R_AXE && my_game->player_cards[i] <= CARD_R_ALL) {
            for(unsigned int j = 0; j < my_game->n_players; j++){
                if( my_game->players_state[j].confident_lvl >= CONFIDENT ){
                    my_move.act = PLAY_REPAIR_CARD;
                    my_move.card = i;
                    my_move.onplayer = j;
                }
            }
        }
    }
    return 0;
}


/**
 * @brief plays the strategy to break a tool
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int play_break_card(struct move my_move, struct move const previous_moves[], size_t n_moves){
    for(unsigned int i=0; i < my_game->n_player_cards; i++){ //the player look at his n_player_cards hand
        if (my_game->player_cards[i] >= CARD_B_AXE && my_game->player_cards[i] <= CARD_B_LAMP) {
            for(unsigned int j = 0; j < my_game->n_players; j++){
                if( my_game->players_state[j].confident_lvl <= UNCONFIDENT ){
                    my_move.act = PLAY_BREAK_CARD;
                    my_move.card = i;
                    my_move.onplayer = j;
                }
            }
        }
    }
    return 0;
}

/**
 * @brief plays the strategy to discard a card for a honest player
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int play_discard_honest(struct move my_move,  struct move const previous_moves[], size_t n_moves){
    my_move.act = DISCARD;
    enum card_id dc = NO_CARD;
    for(unsigned int i=0; i < my_game->n_player_cards; i++){
        if (my_game->player_cards[i] >= CARD_R_AXE && my_game->player_cards[i] <= CARD_R_ALL){
            if( dc = NO_CARD )
                dc = my_game->player_cards[i];
    }
        else if (my_game->player_cards[i]  >= CARD_V_LINE && my_game->player_cards[i]  <= CARD_R_TURN){
            if( dc = NO_CARD || (dc >= CARD_R_AXE && dc <= CARD_R_ALL) )
                dc = my_game->player_cards[i];
        }
        else if (my_game->player_cards[i] >= CARD_B_AXE && my_game->player_cards[i] <= CARD_B_LAMP){
            if( dc = NO_CARD || (dc >= CARD_R_AXE && dc <= CARD_R_ALL) || (dc  >= CARD_V_LINE && dc  <= CARD_R_TURN) )
            dc = my_game->player_cards[i];
    }
        else if (my_game->player_cards[i] == CARD_BOULDER)
            dc = my_game->player_cards[i];
    }
    my_move.card = dc;
    return 0;
}

/**
 * @brief plays the strategy to discard a card for a sapoteur.
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int play_discard_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    my_move.act = DISCARD;
    enum card_id dc = NO_CARD;
    for(unsigned int i=0; i < my_game->n_player_cards; i++){
        if (my_game->player_cards[i] == CARD_BOULDER){
            if( dc = NO_CARD )
                my_move.card = my_game->player_cards[i];
        }
        else if (my_game->player_cards[i] >= CARD_B_AXE && my_game->player_cards[i] <= CARD_B_LAMP){
            if(dc = NO_CARD || dc == CARD_BOULDER)
                my_move.card = my_game->player_cards[i];
        }
        else if (my_game->player_cards[i]  >= CARD_V_LINE && my_game->player_cards[i]  <= CARD_R_TURN){
            if(dc = NO_CARD || dc == CARD_BOULDER || (dc >= CARD_B_AXE && dc <= CARD_B_LAMP) )
                my_move.card = my_game->player_cards[i];
        }
        else if (my_game->player_cards[i] >= CARD_R_AXE && my_game->player_cards[i] <= CARD_R_ALL){
                my_move.card = my_game->player_cards[i];
        }
    }
    my_move.card = dc;
    return 0;
}

/* BOULDER AND PATH STRATEGY */

/**
 * @brief aimed to generate an array that contained the coordinates on the board where a card is placed.
 * @param my_game, a game
 * @param node, a card placed on the board
 * @param objective, the position of an objective
 * @return the length of the array generated
 */
/*HAS TO BE USED ON THE FUNCTION DISTANCE */
static int find_node( game_t my_game, enum card_id tab[]){
    int length= 0;
    for( int x=0; x< game_get_height(my_game); x++){
        for( int y=0; y < game_get_width(my_game); y++ ){
            if( game_get_card(my_game, x, y) != NO_CARD){
                tab[x] = game_get_card(my_game,x, y);
                length++;
            }
        }
    }

    return length;
}

/**
 * @brief aimed to calculate a distance between two position on the board
 * @param my_game, a game
 * @param node, a card placed on the board
 * @param objective, the position of an objective
 * @return 1
 */

unsigned int distance(game_t my_game, struct position node, struct position objective) {
    return 1;
}


/**
 * @brief aimed to place the boulder card just next the the shortest path.
 * @param my_game, a game
 * @return 1
 */

static int place_boulder_card(game_t my_game){
    return 1;
}

/**
 * @brief aimed to play the strategy to place a BOULDER_CARD/
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */



static int play_boulder_card(struct move my_move, struct move const previous_moves[], size_t n_moves ){

    for( int x=0; x< my_game->n_player_cards; x++){
        place_boulder_card(my_game);
    }
}




/**
 * @brief aimed to play the strategy to place a PATH_CARD for a honest player
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */


static int play_path_card_honest(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    return 1;
}

/**
 * @brief aimed to play the strategy to place a PATH_CARD for a honest player
 * @param my_move the move made by the player,
 * @param previous_moves an array of moves,
 * @param n_moves the size of previous_moves
 * @return 0
 */

static int play_path_card_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves){
    return 1;
}



/**********************FUNCTIONS TO PLAY THE GAME**********************/


/**
 * @brief Removes the i_card-th card from the player's hand
 * @param  i_card, a card identifier
 * @return        0 if the card was removed
 *                1 otherwise
 */


int draw_card(enum card_id card){
    if (card != NO_CARD)
        return gameClient_add_card(my_game, card);
    return 1;
}



struct move play(struct move const previous_moves[], size_t n_moves){
    printf("n_move : %lu\n", n_moves);
    struct move prev;
    size_t x;
    size_t y;
    //updating the board
    for( size_t i = 0; i < n_moves; i++ ){
        prev = previous_moves[i];
        x = prev.onto.x;
        y = prev.onto.y;
        switch( prev.act ){
        case ADD_PATH_CARD:
            game_set_dir( my_game, x, y, prev.dir);
             game_set_card( my_game, x, y, prev.card);
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
    /* as all the strategic functions were not implemented, my_move is initialize to avoid a segfault */
    struct move my_move;
    my_move.act = DISCARD;

    strategic_move( previous_moves, n_moves);
    int i_card = my_move.card;
    gameClient_remove_card(my_game, i_card );
    return my_move;
}

/******************FUNCTION TO FREE THE GAME***************/


int finalize(){
    game_free(my_game);
    return 0;
}


/***********************************************************/
