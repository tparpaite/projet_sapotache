#include "../../include/game/game.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// ERROR(str, [args ...])
#define ERROR(STR, ...) fprintf( stderr, __FILE__ ":%d - ERROR : "STR"\n", __LINE__, ##__VA_ARGS__ ); exit(1);

/***********************
 *	struct definitions *
 ***********************/

 struct state_s {
 	unsigned int id;
 	int axe_broken;
 	int lamp_broken;
 	int cart_broken;
 };

struct tile_s {
    enum card_id card;
    enum direction dir;
    int N;
    int S;
    int W;
    int E;
};

struct board_s{
	struct tile_s **tab; // matrix of struct (so not matrix of pointer) to cachefreindly
	unsigned int width;
	unsigned int height;
	size_t n_objectives;
	struct position *objectives;
	struct position start;
	size_t n_holes;
	struct position *holes;
};


struct game_s {
	unsigned int n_cards_total;
	unsigned int n_players;
	struct board_s *board;
	void *extra_data;
	game_free_fct free_extra_data;
	struct state_s *player_states; // player_states[id] = Player(id)'s tools state'
};

/********************************
 * static function declarations *
 ********************************/


static int game_update_cardinalpoints(struct tile_s *t);


/*******************************
 * Public function definitions *
 *******************************/


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
                ){
	game_t this = malloc(sizeof(*this));
	if (this == NULL) {
		ERROR("malloc returned NULL");
	}

	this->n_cards_total = n_cards_total;
	this->n_players = n_players;
	this->extra_data = NULL;
	this->board = malloc(sizeof(*this->board));
	if (this->board == NULL) {
		ERROR("malloc returned NULL");
	}
	this->board->tab = malloc(sizeof(*this->board->tab) * width);
	if (this->board->tab == NULL) {
		ERROR("malloc returned NULL");
	}
	for (size_t i = 0; i < width; i++) {
		this->board->tab[i] = malloc(sizeof(*this->board->tab[i]) * height);
		if (this->board->tab[i] == NULL) {
			ERROR("malloc returned NULL - i == %lu", i);
		}
		for (size_t j = 0; j < height; j++) {
			this->board->tab[i][j].card = NO_CARD;
			this->board->tab[i][j].dir  = NORMAL;
			game_update_cardinalpoints(&this->board->tab[i][j]);
		}
	}
	this->board->width = width;
	this->board->height = height;
	this->board->n_objectives = n_objectives;
	this->board->objectives = malloc(sizeof(*this->board->objectives) * n_objectives);
	if (this->board->objectives == NULL) {
		ERROR("malloc returned NULL");
	}
	for (size_t i = 0; i < n_objectives; i++) {
		this->board->objectives[i] = objectives[i];
	}

	this->board->start = start;
	this->board->n_holes = n_holes;
	this->board->holes = malloc(sizeof(*this->board->holes) * n_holes);
	if (this->board->holes == NULL) {
		ERROR("malloc returned NULL");
	}
	for (size_t i = 0; i < n_holes; i++) {
		this->board->holes[i] = holes[i];
	}

	this->player_states = malloc(sizeof(*this->player_states) * n_players);
	for (size_t i = 0; i < n_players; i++) {
		this->player_states[i].id = i;
		this->player_states[i].axe_broken = 0;
		this->player_states[i].lamp_broken = 0;
		this->player_states[i].cart_broken = 0;
	}


	this->extra_data = extra_data;
	this->free_extra_data = free_extra_data;
	return this;
}

unsigned int game_get_width(game_t this){
    return this->board->width;
}

unsigned int game_get_height(game_t this){
    return this->board->height;
}


/**
 * Get the card at (x, y) on the game board
 * PRECOND :	x < this->board->width
 *              y < this->board->height
 * @param  this game_t game pointer
 * @param  x    size_t x coord
 * @param  y    size_t y coord
 * @return      NO_CARD , if x,y are out of bound
 *              enum card_id the card_id at (x, y), else
 */
enum card_id game_get_card(game_t this, size_t x, size_t y){
	if(x < this->board->width && y < this->board->height){
		return this->board->tab[x][y].card;
	}
	return NO_CARD;
}

/**
 * Set the card at (x, y) on the game board
 * PRECOND :	x < this->board->width
 *              y < this->board->height
 * @param  this game pointer
 * @param  x   size_t x coord
 * @param  y   size_t y coord
 * @param  new_card enum card_id the new card
 * @return 0
 */
int game_set_card(game_t this, size_t x, size_t y, enum card_id new_card){
	assert(x < this->board->width && y < this->board->height);
	this->board->tab[x][y].card = new_card;
	return 0;
}

/**
 * Get the card direction at (x, y) on the game board
 * PRECOND :	x < this->board->width
 *              y < this->board->height
 * @param  this game_t game pointer
 * @param  x    size_t x coord
 * @param  y    size_t y coord
 * @return      NORMAL , if x,y are out of bound
 *              enum direction the card direction at (x, y), else
 */
enum direction game_get_dir(game_t this, size_t x, size_t y){
	if(x < this->board->width && y < this->board->height){
		return this->board->tab[x][y].dir;
	}
	return NORMAL;
}

/**
 * Set the card direction at (x, y) on the game board
 * PRECOND :	x < this->board->width
 *              y < this->board->height
 * @param  this game pointer
 * @param  x   size_t x coord
 * @param  y   size_t y coord
 * @param  new_dir enum direction the new direction
 * @return 0
 */
int game_set_dir(game_t this, size_t x, size_t y, enum direction new_dir){
	assert(x < this->board->width && y < this->board->height);
	this->board->tab[x][y].dir = new_dir;
	return 0;
}

size_t game_get_n_objectives(game_t this){
	return this->board->n_objectives;
}

struct position game_get_ith_objective(game_t this, size_t i){
	return this->board->objectives[i];
}

struct position game_get_start(game_t this){
	return this->board->start;
}

size_t game_get_n_holes(game_t this){
	return this->board->n_holes;
}

struct position game_get_ith_hole(game_t this, size_t i){
	return this->board->holes[i];
}



size_t game_set_n_objectives(game_t this, size_t new_n){
	this->board->n_objectives = new_n;
	return 0;
}

/**
 * PRECOND 	i < game_set_n_objectives(this)
 * @param  this          [description]
 * @param  i             [description]
 * @param  new_objective [description]
 * @return               [description]
 */
int game_set_ith_objective(game_t this, size_t i, struct position new_objective){
	this->board->objectives[i] = new_objective;
	return 0;
}

int game_set_start(game_t this, struct position new_start){
	this->board->start = new_start;
	return 0;
}

int game_set_n_holes(game_t this, size_t new_n){
	this->board->n_holes = new_n;
	return 0;
}

int game_set_ith_hole(game_t this, size_t i, struct position new_hole){
	this->board->holes[i] = new_hole;
	return 0;
}


unsigned int game_get_n_cards_total(game_t this){
	return this->n_cards_total;
}

unsigned int game_get_n_players(game_t this){
	return this->n_players;
}

int game_set_n_cards_total(game_t this, unsigned int new_n){
	this->n_cards_total = new_n;
	return 0;
}

int game_set_n_players(game_t this, unsigned int new_n){
	this->n_players = new_n;
	return 0;
}


void * game_get_extra_data(game_t this){
	return this->extra_data;
}

int game_free(game_t this){
	for (size_t i = 0; i < this->board->width; i++) {
		free(this->board->tab[i]);
	}
    free(this->board->tab);
    free(this->board->objectives);
    free(this->board->holes);
	free(this->player_states);
	free(this->board);
    this->free_extra_data(this->extra_data);
    free(this);
    return 0;
}


int game_player_axe_is_broken(game_t this, unsigned int id){
	return this->player_states[id].axe_broken;
}
int game_player_lamp_is_broken(game_t this, unsigned int id){
	return this->player_states[id].lamp_broken;
}
int game_player_cart_is_broken(game_t this, unsigned int id){
	return this->player_states[id].cart_broken;
}

int game_player_break_axe(game_t this, unsigned int id){
	this->player_states[id].axe_broken = 1;
	return 0;
}
int game_player_repair_axe(game_t this, unsigned int id){
	this->player_states[id].axe_broken = 0;
	return 0;
}
int game_player_break_lamp(game_t this, unsigned int id){
	this->player_states[id].lamp_broken = 1;
	return 0;
}
int game_player_repair_lamp(game_t this, unsigned int id){
	this->player_states[id].lamp_broken = 0;
	return 0;
}
int game_player_break_cart(game_t this, unsigned int id){
	this->player_states[id].cart_broken = 1;
	return 0;
}
int game_player_repair_cart(game_t this, unsigned int id){
	this->player_states[id].cart_broken = 0;
	return 0;
}

int game_print(game_t this){

}

static int print_positions(struct board_s *board,size_t x,size_t y){
    if (board->start.x == x && board->start.y == y) {
        printf(" > ");
        return 1;
    }
    for (size_t i = 0; i < board->n_objectives; i++) {
        if (board->objectives[i].x == x && board->objectives[i].y == y) {
            printf(" $ ");
            return 2;
        }
    }
    for (size_t i = 0; i < board->n_holes; i++) {
        if (board->holes[i].x == x && board->holes[i].y == y) {
            printf(" $ ");
            return 3;
        }
    }
    return 0;
}


int game_print_board(game_t this){
    for (size_t i = 0 ; i < this->board->width ; i++) {
        for (size_t j = 0; j < this->board->height ; j++) {
            if (!print_positions(this->board, i, j)) {
                switch (this->board->tab[i][j].card) {
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
                    if (this->board->tab[i][j].dir == REVERSED) {
                        printf(" |-");
                    }else {
                        printf("-| ");
                    }
                    break;
                case CARD_H_CROSS:
                    if (this->board->tab[i][j].dir == REVERSED) {
                        printf("-,-");
                    }else {
                        printf("-'-");
                    }
                    break;
                case CARD_X_CROSS:
                    printf("-|-");
                    break;
                case CARD_L_TURN:
                    if (this->board->tab[i][j].dir == REVERSED) {
                        printf(" '-");
                    }else {
                        printf("-, ");
                    }
                    break;
                case CARD_R_TURN:
                    if (this->board->tab[i][j].dir == REVERSED) {
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
	return 0;
}


/*******************************
 * Static function definitions *
 *******************************/

 static int game_update_cardinalpoints(struct tile_s *t) {
     switch (t->card){
     case NO_CARD:
         t->N = 1; t->S = 1; t->W = 1; t->E = 1; break;
     case CARD_V_LINE:
         t->N = 1; t->S = 1; t->W = 0; t->E = 0; break;
     case CARD_H_LINE:
         t->N = 0; t->S = 0; t->W = 1; t->E = 1; break;
     case CARD_V_CROSS:
         if(t->dir == NORMAL){
             t->N = 1; t->S = 1; t->W = 1; t->E = 0; break;
         }
         else{
             t->N = 1; t->S = 1; t->W = 0; t->E = 1; break;
         }
     case CARD_H_CROSS:
         if(t->dir == NORMAL){
             t->N = 1; t->S = 0; t->W = 1; t->E = 1; break;
         }
         else{
             t->N = 0; t->S = 1; t->W = 1; t->E = 1; break;
         }
     case CARD_L_TURN:
         if(t->dir == NORMAL){
             t->N = 0; t->S = 1; t->W = 1; t->E = 0; break;
         }
         else{
             t->N = 1; t->S = 0; t->W = 0; t->E = 1; break;
         }
     case CARD_R_TURN:
         if(t->dir == NORMAL){
             t->N = 0; t->S = 1; t->W = 0; t->E = 1; break;
         }
         else{
             t->N = 1; t->S = 0; t->W = 1; t->E = 0; break;
         }
     case CARD_X_CROSS:
         t->N = 1; t->S = 1; t->W = 1; t->E = 1; break;
     case CARD_D_END:
         t->N = 0; t->S = 0; t->W = 0; t->E = 0; break;
     default:
	 	// Shouldn't never got here
         return 1;
     }
     return 0;
 }
