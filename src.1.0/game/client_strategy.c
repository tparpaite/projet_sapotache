#include "../../include/game/sapotache_interface.h"
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
    enum p_type type; // Type de nain
    unsigned int n_cards_total;
    size_t n_player_cards;
    enum card_id *player_cards;
    unsigned int n_players;
    struct board_s board;
    struct state_s *players_state; // players_state[id] = Player(id)'s tools state'
};



struct game_s my_game;

/***********************HEADERS*********************************/

static int remove_card(size_t i_card);
struct move copy_move_afromb( struct move a, struct move b );

static int ia_update_cardinalpoints(struct tile_s *t);
static int ia_check_cardinalpoints(struct move m);



/*  returns the strategic move that the player should play acording to the actual situation */

/*
static struct move strategic_move( struct move const previous_moves[], size_t n_moves );
static struct move strategy_honest(struct move const previous_moves[], size_t n_moves );
static struct move strategy_sapoteur(struct move const previous_moves[], size_t n_moves );


static int  play_break_card(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int  play_repair_card(struct move my_move, struct move const previous_moves[], size_t n_moves);
static int play_path_card_honest(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int play_path_card_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves);
static int play_boulder_card(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int play_discard_honest(struct move my_move,  struct move const previous_moves[], size_t n_moves);
static int play_discard_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves );

*/

/***********************UTILITARIAN FUNCTIONS*********************/


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
    my_game.id = id;
    my_game.type = type;
    my_game.n_cards_total = n_cards_total;
    my_game.n_player_cards = n_player_cards;
    // TODO quelle taille pour le tableau player_cards?
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
    for (size_t i = 0 ; i < width ; i++) {
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
    int h = my_game.board.height;
    int l = my_game.board.width;
    if( x > 0 )
        above = (my_game.board.tab[x -1][y]);
    else
        above.card = NO_CARD;
    if( x < h - 1 )
        below = (my_game.board.tab[x +1][y]);
    else
        below.card = NO_CARD;
    if( y > 0 )
        left = (my_game.board.tab[x][y - 1]);
    else
        left.card = NO_CARD;
    if( y < l - 1 )
        right = (my_game.board.tab[x][y + 1]);
    else
        right.card = NO_CARD;

    ia_update_cardinalpoints( &above );
    ia_update_cardinalpoints( &below );
    ia_update_cardinalpoints( &right );
    ia_update_cardinalpoints( &left );

    switch (m.card){
        //case NO_CARD: 
        //break;
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

/*

static int  play_repair_card(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    
    for(unsigned int i=0; i < my_game.n_player_cards; i++){ //the player look at his n_player_cards hand
        if (my_game.player_cards[i] >= CARD_R_AXE && my_game.player_cards[i] <= CARD_R_ALL) {
            for(unsigned int j = 0; j < my_game.n_players; j++){
                if( my_game.players_state[j].confident_lvl >= CONFIDENT ){
                    my_move.act = PLAY_REPAIR_CARD;
                    my_move.card = i;
                    my_move.onplayer = j;
                        }
            }
        }
    }


        
        static int play_break_card(struct move my_move, struct move const previous_moves[], size_t n_moves){
                for(unsigned int i=0; i < my_game.n_player_cards; i++){ //the player look at his n_player_cards hand
            if (my_game.player_cards[i] >= CARD_B_AXE && my_game.player_cards[i] <= CARD_B_ALL) {
                for(unsigned int j = 0; j < my_game.n_players; j++){
                    if( my_game.players_state[j].confident_lvl <= UNCONFIDENT ){
                        my_move.act = PLAY_BREAK_CARD;
                        my_move.card = i;
                        my_move.onplayer = j;
                    }
                }
        }
}    
}
        */

        /*
static int play_path_card_honest(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int play_path_card_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves);
static int play_boulder_card(struct move my_move, struct move const previous_moves[], size_t n_moves );
static int play_discard_honest(struct move my_move,  struct move const previous_moves[], size_t n_moves);
static int play_discard_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves );
*/

/*

    
    int i_card = 0;
    
   

        if (my_move.card >= CARD_V_LINE && my_move.card <= CARD_R_TURN) {
            
        }else if (my_move.card >= CARD_B_AXE && my_move.card <= CARD_B_CART) {
            
        }else 
        }else if (my_move.card == CARD_BOULDER) {
            
        }
        my_move.card = strategic_card(previous_moves, n_moves );
    }
    my_move.card = i_card; //TODO : id de la carte jugée bonne à jouer
    remove_card(i_card);

*/
    
/*
static int strategy_honest(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    struct move my_move;
    my_move.player = my_game.id;

    my_move = play_path_card_honest( previous_moves, n_moves);
    if( ia_check_cardinalpoints(my_move)){
        my_move = play_repair_card( previous_moves, n_moves);
        if( ia_check_cardinalpoints(my_move)){
            my_move = play_break_card(previous_moves, n_moves);
            if( ia_check_cardinalpoints(my_move)){
                my_move = play_discard_honest(previous_moves, n_moves);
            }
        }
    }
    return my_move;
}

static int strategy_sapoteur(struct move my_move, struct move const previous_moves[], size_t n_moves ){
    my_move = play_boulder_card(previous_moves, n_moves);
    if( ia_check_cardinalpoints(my_move)){
        my_move = play_break_card(previous_moves, n_moves);
        if( ia_check_cardinalpoints(my_move)){
            my_move = play_path_card_sapoteur( previous_moves, n_moves);
            if( ia_check_cardinalpoints(my_move)){
                my_move = play_discard_sapoteur(previous_moves, n_moves);
                if( ia_check_cardinalpoints(my_move)){
                    my_move = play_repair_card( previous_moves, n_moves);
                }
            }
        }
    }
    return my_move;
}

*/

/*  returns the strategic move that the player should play acording to the actual situation */

/*
static struct move strategic_move(struct move const previous_moves[], size_t n_moves ){
    struct move my_move;
    if(my_game.type == HONEST )
        strategy_honest( my_move, previous_moves, n_moves );
    if(my_game.type == SAPOTEUR )
        strategy_sapoteur( my_move, previous_moves, n_moves );
    return my_move;
}

*/



/**********************FUNCTIONS TO PLAY THE GAME**********************/


int draw_card(enum card_id card){
    if( card != NO_CARD ){
        my_game.player_cards[my_game.n_player_cards] = card;
        my_game.n_player_cards++;
    }
    return 0;
}


/**
 * Remove the i_card-th to the player hand
 * @param  i_card card index
 * @return        0 if the card was removed
 *                1 else
 */


static int remove_card(size_t i_card){ // TODO : On retourne jamais 1?
    assert(i_card < my_game.n_player_cards);
    my_game.n_player_cards--;
    for (size_t i = i_card; i < my_game.n_player_cards; i++) {
        my_game.player_cards[i] = my_game.player_cards[i+1];
    }
    return 0;
}



struct move play(struct move const previous_moves[], size_t n_moves){
    printf("n_move : %lu\n", n_moves); // TODO : FIX n_move == 0 !!
    //update the board
    for( size_t i = 0; i < n_moves; i++ ){
        printf("%lu mississipi\n", i);
        switch( previous_moves[i].act ){
        case ADD_PATH_CARD:
            my_game.board.tab[previous_moves[i].onto.x][previous_moves[i].onto.y].dir = previous_moves[i].dir;
        case PLAY_BOULDER_CARD:
            my_game.board.tab[previous_moves[i].onto.x][previous_moves[i].onto.y].card = previous_moves[i].card;
            // TODO: retirer les cartes de la main des joueurs.
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

    /* Quelle carte jouer? */
    //strategic_move( previous_moves, n_moves);
    remove_card( 0); //là pour compiler parce que tout est commenté
    ia_check_cardinalpoints(my_move);//idem
    return my_move;
}

/******************FUNCTIONS TO FREE THE GAME***************/

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


/***********************************************************/
