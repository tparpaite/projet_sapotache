#define _GNU_SOURCE

#include "adt_list.h"
#include "adt_coord.h"
#include "parser.h"
#include "server.h"
#include "sapotache_interface.h"
#include <assert.h>
#include <time.h>
#include <dlfcn.h>
#include <link.h>
#include "game_server.h"
#include "game.h"


#define NB_GOLD_CARD1 16
#define NB_GOLD_CARD2 8
#define NB_GOLD_CARD3 4


struct cardinal_point_s{
    size_t x;
    size_t y;
    int N;
    int S;
    int W;
    int E;
};

typedef int (*client_initialize_t)(unsigned int id,
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
                                   unsigned int n_players);


struct gold_cards{
    int n_gold_cards;
    int gold_tab[NB_GOLD_CARD1 + NB_GOLD_CARD2 + NB_GOLD_CARD3];
};

struct gold_cards gold_cards_g;


/***** GAME / INITIALIZATION *****/
/**
 * @brief Creates a new_game and initialize it
 * @param board_path, the path which indicates where is the file to parse
 * @param n_players, number of players
 * @param client_library_paths_a, an array indicating the library to use (.so) for each client
 * @return the data structure representing a new game
 */
static game_t server_new_game(char *board_path, int n_players, char **client_library_paths_a);

/**
 * @brief Free memory of a game
 * @param g, a game
 * @return 0 if game was successfuly freed
 *         1 otherwise
 */
static int server_free_game(game_t g);

/**
 * @brief Deals the initial cards to each player
 * @param g, a game
 * @return player_cards, an array of card_id containing the initial cards of every player
 *
 */
static enum card_id *server_deal_initial_cards(game_t g);

/**
 * @brief Fills an array of booleans describing whether player is a saboteur or not (related to index)
 * @param n_players, the current number of players
 * @param saboteurs, an array of integers
 * @return  0
 *
 */
static int server_generate_saboteurs(int n_players, int *saboteurs);

/**
 * @brief Draws a card from the deck
 * @param g, a game
 * @return card, the card which was removed from the deck
 *
 */
static enum card_id server_pop_card(game_t g);

/**
 * @brief Draws a card from the deck and adds it to the player's hand of cards
 * @param g, a game
 * @param player_id, the identifier from the player in question
 * @return 0
 *
 */
static int server_draw_card(game_t g, int player_id);


/***** PLAYER *****/
/**
 * @brief Creates a new player from a client (a library)
 * @param client_library_path, the path to the client's library
 * @param id, an integer indicating whether the new client will be a sapoteur
 * @param type, the type of the new player
 * @param n_player_cards, the number of cards of the new player
 * @param player_cards, the array containing the cards of the new player
 * @param g, a game
 * @return p, the new player
 *
 */
// static player_t server_new_player(char *client_library_path, int id, enum p_type type, size_t n_player_cards, enum card_id *player_cards, game_t g);


/**
 * @brief Adds a card to p's hand of cards
 * @param g, a game
 * @param id, the identifier of a player
 * @param card, the card to add
 * @return 0
 *
 */
static int server_add_player_card(game_t g, unsigned int id, enum card_id card);

/**
 * @brief Removes a card from p's hand of cards
 * @param id, the identifer of a player
 * @param card, the card to remove
 * @param g, a game
 * @return 0 if it was correctly removed
 *         1 if the card was not found in his hand
 *
 */
static int server_remove_player_card(unsigned int id, enum card_id card, game_t g);

/**
 * @brief Kicks a player out of the game
 * @param g, the game
 * @param player_id, the identifier of the player
 * @param s, the error message to display
 * @return 0
 *
 */
static int server_kick_player(game_t g, int player_id, char *s);

/**
 * @brief Gives the amount of cards to deal to each player during the game
 * @param n_players, the number of player in the game
 * @return n_player_cards, the size hand
 *
 */
static int server_n_player_cards(int n_players);


/***** MAIN_LOOP *****/
/**
 * @brief Updates the array of previous moves
 * @param g, a game
 * @param m, the move to add to the array of previous moves
 * @return 0
 *
 */
static int server_update_previousmoves(game_t g, struct move m);

/**
 * @brief Launches the game
 * @param g, the game played
 * @return winner, the type of player that won the game * @brief
 *
 */
static enum p_type server_main_loop(game_t g);

/**
 * @brief Plays the move given
 * @param g, a game
 * @param m, the move to be played
 * @return 0 if the move was played correctly
 *         1 otherwise
 *
 */
static int server_play_move(game_t g, struct move m);
static int server_play_boulder(game_t g, struct move m);
static int server_play_pathcard(game_t g, struct move m);

/**
 * @brief Plays a move that implies breaking something
 * @param g, a game
 * @param m, the move
 * @return 0 if the move was played correctly
 *         1 otherwise
 *
 */
static int server_play_breakcard(game_t g, struct move m);

/**
 * @brief Plays a move that implies reparing something
 * @param g, a game
 * @param m, the move
 * @return 0 if the move was played correctly
 *         1 otherwise
 *
 */
static int server_play_repaircard(game_t g, struct move m);

/**
 * @brief Checks whether a move is valid
 * @param g, a game
 * @param m, the move to be played
 * @return 0 if the move is valid
 *         1 if the card to be played is not in the hand of the player
 *         2 if the position of the card to be played was out of range
 *         3 if there already is a card where we want to put one
 *         4 if the card don't match the ones that are next to where we want to put it
 *         5 if the move implied using a break/repair card on a player who does not exist
 *         6 if there was an unvalid action
 *
 */
static int server_move_isnotvalid(game_t g, struct move m);

/**
 * @brief updates the values of the cardinal points of a tile
 * @param t a pointer to a tile
 * @return 0 if it was successfully updated
 *         1 otherwise
 *
 */
static int server_update_cardinalpoints(game_t g, struct cardinal_point_s *t);
// static int server_is_start(game_t g, int x, int y);

/**
 * @brief Checks if it is possible to make the move m considering the cardinal points of the cards next to the position we want where we want to put the card
 * @param g, a game
 * @param m, a move
 * @return 0 if the move is possible considering cardinal points
 *         1 otherwise
 *
 */
static int server_check_cardinalpoints(game_t g, struct move m);
static int server_check_special_positions(game_t g, struct move m);


/***** GOLD *****/
/**
 * @brief Initializes the array of gold from gold_cards_g
 * @return 0
 *
 */
static int initialize_gold_tab( void );

/**
 * @brief Fills the array of gold from gold_cards_g
 * @return 0
 *
 */
static int fill_gold_tab( void );

/**
 * @brief Gives a random number of gold nuggets
 * @return gold, a number between 1 and 3
 *
 */
static int random_gold( void );

/**
 * @brief Checks whether the position of a move corresponds to an end position (objective)
 * @param g, a game
 * @param previous_move_i, the index of the move in question in previous_moves
 * @param objectives_j, the index of the end position in objectives
 * @return 0 if it does correspond to the given end position
 *         1 otherwise
 *
 */
static int position_comparison_end( game_t g, int previous_move_i, int objectives_j );

/**
 * @brief Gives a pointer to the player who won the game
 * @param g, a game
 * @return w_player, the winner of the game
 *         NULL if nobody won
 *
 */
static int winner( game_t g);

/**
 * @brief Distributes gold nuggets at the end of a game
 * @param g, a game
 *
 */
static void gold_distribution( game_t g);


/***** UTILS *****/
/**
 * @brief Makes an array out of l
 * @param l, a list of positions
 * @return res, an array of positions
 *
 */
static struct position *server_list_to_array(list_t l);

/**
 * @brief Makes a position out of coordonates
 * @param c, a pair of coordinates
 * @return p, a position
 *
 */
static struct position server_coord_to_position(coord_t c);

/**
 * @brief Displays the player of a given move : used to debug
 * @param m, a move
 * @return 0
 *
 */
static int print_move(struct move m);

struct cardinal_point_s **cardinalpoints_mat;

int server_init_cardinal_points(game_t g){
    cardinalpoints_mat = malloc(sizeof(*cardinalpoints_mat) * game_get_width(g));
    for (size_t i = 0; i < game_get_width(g); i++) {
        cardinalpoints_mat[i] = malloc(sizeof(**cardinalpoints_mat) * game_get_height(g));
        for (size_t j = 0; j < game_get_height(g); j++) {
            cardinalpoints_mat[i][j].x = i;
            cardinalpoints_mat[i][j].y = j;
            server_update_cardinalpoints(g, &cardinalpoints_mat[i][j]);
        }
    }
    return 0;
}

static game_t server_new_game(char *board_path, int n_players, char **client_library_paths_a) {
    srand(time(NULL));

    /* Opening stream */
    FILE *f = fopen(board_path, "r");
    if (f == NULL) {
        fprintf(stderr, "*** %s : cannot open file %s\n", "server_new_game", board_path);
        exit(1);
    }

    memory_t mem = parse_memory(f);

    unsigned int width = memory_get_width(mem);
    unsigned int height = memory_get_height(mem);
    size_t n_objectives = memory_get_n_objectives(mem);
    size_t n_holes = memory_get_n_holes(mem);
    unsigned int n_cards_total = memory_get_n_cards(mem);

    struct position const *objectives = server_list_to_array(memory_get_objectives(mem));
    struct position start = server_coord_to_position(memory_get_start(mem));
    struct position const *holes = server_list_to_array(memory_get_holes(mem));

    list_t const deck = list_copy(memory_get_l_deck(mem));

    game_t g = gameServer_new(
        width,
        height,
        n_objectives,
        objectives,
        start,
        n_holes,
        holes,
        n_cards_total,
        n_players,
        deck
    );

    /* Creating game */
    fill_gold_tab();

    /* Creation of players */

    /* Initializing previous moves */
    int saboteurs[n_players];
    for (size_t i = 0; i < n_players; i++) {
        saboteurs[i] = 0;
    }
    server_generate_saboteurs(n_players, saboteurs);

    int n_player_cards = server_n_player_cards(n_players);

    for (unsigned int i = 0 ; i < n_players ; ++i) {
        enum card_id *player_cards;
        player_cards = server_deal_initial_cards(g);
        void *handler = dlmopen(LM_ID_NEWLM, client_library_paths_a[i], RTLD_NOW);
        gameServer_player_init(
            g,
            i,
            handler,
            dlsym(handler, "play"),
            dlsym(handler, "draw_card"),
            saboteurs[i],
            n_player_cards,
            player_cards
        );
        client_initialize_t c_initialize = dlsym(handler, "initialize");
        /* This triggers a warning "ISO C forbids initialization between function pointer and ‘void *’".
           It can't be avoided because it is an old function and so it returns a void *... */
        if (c_initialize == NULL) {
            perror("Error loading c_initialize");
            exit(1);
        }
        c_initialize(
            i,
            gameServer_player_get_type(g, i),
            width,
            height,
            start,
            n_objectives,
            objectives,
            n_holes,
            holes,
            n_cards_total,
            n_player_cards,
            player_cards,
            n_players
        );
    }

    return g;
}


static int server_free_game(game_t g) {
    for (size_t i = 0; i < game_get_n_players(g); i++) {
        dlclose(gameServer_player_get_handler(g, i));
    }
    return game_free(g);
}


static struct position *server_list_to_array(list_t l) {
    int n = list_size(l);
    struct position *res = malloc(n * sizeof(struct position));

    int i = 0;
    list_begin(l);

    while (!list_isend(l)) {
        coord_t c = list_getdata(l);
        res[i] = server_coord_to_position(c);
        coord_free(c);
        ++i;
        list_next(l);
    }

    return res;
}


static struct position server_coord_to_position(coord_t c) {
    struct position p;
    p.x = coord_getX(c);
    p.y = coord_getY(c);
    return p;
}





static int server_remove_player_card(unsigned int id, enum card_id card, game_t g) {
    enum card_id new;
    for (size_t i = 0 ; i < gameServer_player_get_n_cards(g, id) ; ++i) {
        /* Removing the card */
        if (gameServer_player_get_ith_cards(g, id, i ) == card) {
            /* Left shift */
            gameServer_player_set_n_cards(g, id, gameServer_player_get_n_cards(g, id) -1);
            for (size_t j = i ; j < gameServer_player_get_n_cards(g, id) ; ++j){
                new = gameServer_player_get_ith_cards(g, id, j+1);
             gameServer_player_set_ith_cards(g, id, j, new);
        }
            return 0;
        }
    }

    perror("Card wasn't found");
    return 1;
}


/* PRE : p->player_cards is an array of size n_cards_total */
static int server_add_player_card( game_t g, unsigned int id, enum card_id card) {
    if (card != NO_CARD) {
        size_t n_cards = gameServer_player_get_n_cards(g, id);
        gameServer_player_set_ith_cards(g, id, n_cards , card);
         gameServer_player_set_n_cards(g, id, gameServer_player_get_n_cards(g, id) );
    }

    return 0;
}


static int server_n_player_cards(int n_players) {
    if (n_players >= 3 && n_players <= 5)
        return 6;
    else if (n_players >= 6 && n_players <= 7)
        return 5;
    else /* 8 to 10 players */
        return 4;
}



static int server_generate_saboteurs(int n_players, int *saboteurs) {
    int n_saboteurs;

    if (n_players >= 3 && n_players <= 4)
        n_saboteurs = 1;
    else if (n_players >= 5 && n_players <= 6)
        n_saboteurs = 2;
    else if (n_players >= 7 && n_players <= 9)
        n_saboteurs = 3;
    else /* 10 players */
        n_saboteurs = 4;

    int rank;
    int i_rank;
    int j;

    /* pseudo random */
    for (int i = 0 ; i < n_saboteurs ; ++i) {
        rank = rand()%(n_players - i - 1);
        i_rank = 0;
        j = 0;

        while (i_rank < rank && j < n_players) {
            if (saboteurs[i] == 0)
                i_rank++;
            j++;
        }

        saboteurs[j] = 1;
    }

    return 0;
}



static enum card_id *server_deal_initial_cards(game_t g) {
    int n_player_cards = server_n_player_cards( game_get_n_players(g) );
    enum card_id *player_cards = malloc( game_get_n_cards_total(g) * sizeof(enum card_id));

    for (int i = 0 ; i < n_player_cards ; ++i) {
        player_cards[i] = server_pop_card(g);
    }

    return player_cards;
}


static enum card_id server_pop_card(game_t g) {
    return gameServer_pop_card(g);
}


static int server_draw_card(game_t g, int player_id) {
    enum card_id card = server_pop_card(g);

    /* Updating server */
    server_add_player_card(g, player_id, card);

    /* Updating client */
    gameServer_player_draw_card(g, player_id, card);

    return 0;
}


int server_start_game(char *board_path, int n_players, char **client_library_paths_a) {
    game_t g = server_new_game(board_path, n_players, client_library_paths_a);
    server_init_cardinal_points(g);
    enum p_type winner = server_main_loop(g);

    if (winner == SAPOTEUR)
        printf("SAPOTEUR won\n");
    else
        printf("HONEST won\n");

    server_free_game(g);

    return 0;
}


static int server_update_previousmoves(game_t g, struct move m) {
    struct move prev_m;
    size_t n_move = gameServer_get_n_moves(g);
    /* First turn */
    if (gameServer_get_n_moves(g) < game_get_n_players(g) ) {
        gameServer_player_set_ith_previous_moves(g, n_move, m);
        gameServer_inc_n_moves(g);
        return 0;
    }

    /* After first turn */
    /* Left shift */
    for (unsigned int i = 0 ; i < game_get_n_players(g) - 1 ; ++i){
        prev_m = gameServer_get_ith_previous_move(g, i+1);
        gameServer_player_set_ith_previous_moves(g, i, prev_m);
    }
    gameServer_player_set_ith_previous_moves(g, n_move, m);
    return 0;
}


static int server_kick_player(game_t g, int player_id, char *s) {
    perror(s);

    gameServer_player_kick(g, player_id);

    /* Don't need to update previous moves (first turn) */
    if (gameServer_get_n_moves(g) < game_get_n_players(g))
        return 0;

    /* Removing the player from previous moves */
    struct move temp;
    gameServer_dec_n_moves(g);
    for (unsigned int i = 0 ; i < gameServer_get_n_moves(g) ; ++i){
        temp = gameServer_get_ith_previous_move (g,i+1);
        gameServer_player_set_ith_previous_moves(g, i, temp);
    }
    return 0;
}


/* Initialize the values of the fields N, S, E and W of a tile based
   on the kind of card it represents */

static int server_update_cardinalpoints(game_t g, struct cardinal_point_s *t) {
    enum direction dir = game_get_dir(g, t->x, t->y);
    switch (game_get_card(g, t->x, t->y)){
    case NO_CARD:
    case CARD_BOULDER:
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
        if (dir == NORMAL) {
            t->N = 1;
            t->S = 1;
            t->W = 1;
            t->E = 0;
            break;
        }
        else {
            t->N = 1;
            t->S = 1;
            t->W = 0;
            t->E = 1;
            break;
        }
    case CARD_H_CROSS:
        if (dir == NORMAL) {
            t->N = 1;
            t->S = 0;
            t->W = 1;
            t->E = 1;
            break;
        }
        else {
            t->N = 0;
            t->S = 1;
            t->W = 1;
            t->E = 1;
            break;
        }
    case CARD_L_TURN:
        if (dir == NORMAL) {
            t->N = 0;
            t->S = 1;
            t->W = 1;
            t->E = 0;
            break;
        }
        else {
            t->N = 1;
            t->S = 0;
            t->W = 0;
            t->E = 1;
            break;
        }
    case CARD_R_TURN:
        if (dir == NORMAL) {
            t->N = 0;
            t->S = 1;
            t->W = 0;
            t->E = 1;
            break;
        }
        else {
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

static int server_is_start(game_t g, int x, int y) {
    return ( game_get_start(g).x == x) && (game_get_start(g).y == y);
}


/* Returns 0 if the move can be executed : the given card can be placed
  at the given set of the board.
  Returns 1 otherwise. */
static int server_check_cardinalpoints(game_t g, struct move m) {
    /* Storing the cards situated below, beside
       and above the one we would like to play */
    struct cardinal_point_s above;
    struct cardinal_point_s below;
    struct cardinal_point_s left;
    struct cardinal_point_s right;

    struct cardinal_point_s sentinel = {game_get_width(g), game_get_height(g), 1, 1, 1, 1};

    int x = m.onto.x;
    int y = m.onto.y;
    int h = game_get_height(g);
    int l = game_get_width(g);

    if(x > 0)
        left = cardinalpoints_mat[x-1][y];
    else
        left = sentinel;

    if (x < h - 1)
        right = cardinalpoints_mat[x+1][y];
    else
        right = sentinel;

    if (y > 0)
        below = cardinalpoints_mat[x][y-1];
    else
        below = sentinel;

    if (y < l - 1)
        above = cardinalpoints_mat[x][y+1];
    else
        above = sentinel;

    /* Checking if the card is on a path (adjacent to start or another path card) */
    if (game_get_card(g, above.x, above.y) == NO_CARD && !server_is_start(g, x - 1, y) &&
        game_get_card(g, below.x, below.y) == NO_CARD && !server_is_start(g, x + 1, y) &&
        game_get_card(g, left.x, left.y) == NO_CARD && !server_is_start(g, x, y - 1) &&
        game_get_card(g, right.x, right.y) == NO_CARD && !server_is_start(g, x, y + 1)) {
        return 1;
    }

    /* Checking cardinal points */
    switch (m.card) {
    case CARD_V_LINE:
        if (above.S == 1 && below.N == 1 &&
            (game_get_card(g, left.x, left.y) == NO_CARD || left.E == 0) &&
            (game_get_card(g, right.x, right.y) == NO_CARD || right.W == 0))
            return 0;
        break;
    case CARD_H_LINE:
        if ((game_get_card(g, above.x, above.y) == NO_CARD || above.S == 0) &&
            (game_get_card(g, below.x, below.y) == NO_CARD || below.N == 0) &&
            left.E == 1 && right.W == 1)
            return 0;
        break;
    case CARD_V_CROSS:
        if (m.dir == NORMAL && above.S == 1 && below.N == 1 && left.E == 1 &&
            (game_get_card(g, right.x, right.y) == NO_CARD || right.W == 0))
            return 0;
        if (m.dir == REVERSED && above.S == 1 && below.N == 1 && right.W == 1 &&
            (game_get_card(g, left.x, left.y) == NO_CARD || left.E == 0))
           return 0;
        break;
    case CARD_H_CROSS:
        if (m.dir == NORMAL && above.S == 1 && right.W == 1 && left.E == 1 &&
            (game_get_card(g, below.x, below.y) == NO_CARD || below.N == 0))
            return 0;
        if (m.dir == REVERSED && below.N == 1 && right.W == 1 && left.E == 1 &&
            (game_get_card(g, above.x, above.y) == NO_CARD || above.S == 0))
            return 0;
        break;
    case CARD_L_TURN:
        if (m.dir == NORMAL && below.N == 1 && left.E == 1 &&
            (game_get_card(g, above.x, above.y) == NO_CARD || above.S == 0) &&
            (game_get_card(g, right.x, right.y) == NO_CARD || right.W == 0))
            return 0;
        if (m.dir == REVERSED && above.S == 1 && right.W == 1 &&
            (game_get_card(g, below.x, below.y) == NO_CARD || below.N == 0) &&
            (game_get_card(g, left.x, left.y) == NO_CARD || left.E == 0))
            return 0;
        break;
    case CARD_R_TURN:
        if (m.dir == NORMAL && below.N == 1 && right.W == 1 &&
            (game_get_card(g, above.x, above.y) == NO_CARD || above.S == 0) &&
            (game_get_card(g, left.x, left.y) == NO_CARD || left.E == 0))
            return 0;
        if (m.dir == REVERSED && above.S == 1 && left.E == 1 &&
            (game_get_card(g, below.x, below.y) == NO_CARD || below.N == 0) &&
            (game_get_card(g, right.x, right.y) == NO_CARD || right.W == 0))
            return 0;
        break;
    case CARD_X_CROSS:
        if (below.N == 1 && above.S == 1 && right.W == 1 && left.E == 1)
            return 0;
        break;
    default:
        return 1;
    }

    return 1;
}


/* Return 0 if the move can be executed, 1 otherwise */
static int server_check_special_positions(game_t g, struct move m) {
    /* Checking if we are not playing on an objectives */
    for (size_t i = 0 ; i < game_get_n_objectives(g) ; ++i) {
        if (game_get_ith_objective(g,i).x == m.onto.x &&
            game_get_ith_objective(g,i).y == m.onto.y)
            return 1;
    }


    /* Checking if we are not playing on a hole */
    for (size_t i = 0 ; i < game_get_n_holes(g) ; ++i) {
        if (game_get_ith_hole(g,i).x == m.onto.x &&
            game_get_ith_hole(g,i).y == m.onto.y)
            return 1;
    }


    /* Checking if we are not playing on start position */
    if (game_get_start(g).x == m.onto.x && game_get_start(g).y == m.onto.y)
        return 1;

    return 0;
}


static int server_move_isnotvalid(game_t g, struct move m) {
    /* Checking if the player got the card in his hand */
    int found = 0;
    size_t n_cards = gameServer_player_get_n_cards( g, m.player );
    for (int i = 0 ; i<n_cards ; ++i) {
        if ( gameServer_player_get_ith_cards(g, m.player, i) ) {
            found = 1;
            break;
        }
    }

    if (found == 0) {
        server_kick_player(g, m.player, "KICK : using card which is not in hand (trying to cheat !)\n");
        return 1;
    }


    /* Then checking whether the move is valid */
    switch (m.act) {
    case ADD_PATH_CARD:
    case PLAY_BOULDER_CARD:
        /* Checking if x and y are in range */
        if (m.onto.x >= game_get_width(g) || m.onto.y >= game_get_height(g) ) {
            server_kick_player(g, m.player, "KICK : x or y is not in range\n");
            return 2;
        }

        /* Checking if the card is not played on a special position (start, objective, hole) */
        if (server_check_special_positions(g, m)) {
            server_kick_player(g, m.player, "KICK : you can't put this card here (special position)\n");
            return 3;
        }

        if (m.act == PLAY_BOULDER_CARD)
            break;

        /* Checking if there is not already a card at the location */
        if (game_get_card(g, m.onto.x, m.onto.y) != NO_CARD) {
            server_kick_player(g, m.player, "KICK : there is already a card at this position\n");
            return 4;
        }

        /* Checking if the card can be played at this location (cardinal points + on a path) */
        if (server_check_cardinalpoints(g, m)) {
            server_kick_player(g, m.player, "KICK : you can't put this card here (invalid cardinals points)\n");
            return 5;
        }

        break;
    case PLAY_BREAK_CARD:
    case PLAY_REPAIR_CARD:
        /* Checking if the player is valid */
        if (m.onplayer >= game_get_n_players(g)) {
            server_kick_player(g, m.player, "KICK : break/repair, invalid player\n");
            return 6;
        }
    case DISCARD:
        break;
    case FAILED: /* Should not happen */
    default:
        server_kick_player(g, m.player, "KICK : FAILED or invalid enum action");
        return 7;
        break;
    }

    /* Move is valid */
    return 0;
}


static enum p_type server_main_loop(game_t g) {
    int cpt = 3;
    struct move *moves = gameServer_get_previous_moves(g);
    size_t n_moves = gameServer_get_n_moves(g);
    /* Play */
    while (cpt >= 0) {
        for (unsigned int i = 0 ; i < game_get_n_players(g) ; ++i) {
            if (cpt <= 0) {
                continue;
            }
            if ( gameServer_player_is_kicked(g, i))
                continue;

            struct move current_move = gameServer_player_play(g, i, moves, n_moves);
            print_move(current_move);

            if (server_move_isnotvalid(g, current_move)) {
                /* Player is kicked in server_move_isnotvalid */
                continue;
            }

            /* Updating previous_moves (for other clients) */
            server_update_previousmoves(g, current_move);

            /* Update the server game with the action related to move */
            server_play_move(g, current_move);

            /* Removing the card to the player */
            server_remove_player_card(i, current_move.card, g);

            /* Drawing a card (pop) and update server and client) */
            server_draw_card(g, i);
        }
        cpt--;
    }

    return HONEST;
}


static int server_play_move(game_t g, struct move m) {
    switch (m.act) {
    case ADD_PATH_CARD:
        server_play_pathcard(g, m);
        break;
    case PLAY_BOULDER_CARD:
        server_play_boulder(g, m);
        break;
    case PLAY_BREAK_CARD:
        server_play_breakcard(g, m);
        break;
    case PLAY_REPAIR_CARD:
        server_play_repaircard(g, m);
        break;
    case DISCARD: /* Discard is done just after */
    case FAILED: /* Should not append because move_isnotvalid is called before */
        break;
    default:
        perror("Invalid enum action");
        return 1;
        break;
    }

    return 0;
}


static int server_play_boulder(game_t g, struct move m) {
    assert(m.act == PLAY_BOULDER_CARD);

    game_set_card(g, m.onto.x, m.onto.y, NO_CARD);
    game_set_dir(g, m.onto.x, m.onto.y, NORMAL);
    server_update_cardinalpoints(g, &cardinalpoints_mat[m.onto.x][m.onto.y]);

    return 0;
}


/* Play a path card */
static int server_play_pathcard(game_t g, struct move m) {
    assert(m.act == ADD_PATH_CARD);

    game_set_card(g, m.onto.x, m.onto.y, m.card);
    game_set_dir(g, m.onto.x, m.onto.y, m.dir);

    /* Don't forget to update cardinal points */
    server_update_cardinalpoints(g, &cardinalpoints_mat[m.onto.x][m.onto.y]);

    return 0;
}


static int server_play_breakcard(game_t g, struct move m) {
    assert(m.act == PLAY_BREAK_CARD);
    switch (m.card) {
    case CARD_B_AXE:
        game_player_break_axe( g, m.onplayer ); //modify broke_axe
        break;
    case CARD_B_LAMP:
        game_player_break_lamp(g, m.onplayer);
        break;
    case CARD_B_CART:
        game_player_break_cart(g, m.onplayer);
        break;
    default:
        perror("server_play_breakcard");
        return 1;
        break;
    }

    return 0;
}


static int server_play_repaircard(game_t g, struct move m) {
    assert(m.act == PLAY_REPAIR_CARD);

    switch (m.card) {
    case CARD_R_AXE:
        game_player_repair_axe(g,m.onplayer);
        break;
    case CARD_R_LAMP:
        game_player_repair_lamp(g, m.onplayer);
        break;
    case CARD_R_CART:
        game_player_repair_cart(g, m.onplayer);
        break;
    case CARD_R_ALL:
      game_player_repair_axe(g, m.onplayer);
      game_player_repair_cart(g,m.onplayer);
      game_player_repair_lamp(g,m.onplayer);
    default:
        perror("server_play_repaircard");
        return 1;
        break;
    }

    return 0;
}


static int print_move(struct move m) {
    printf("---\n");
    printf("player : %u\n", m.player);

    return 0;
}


static int print_positions(game_t g,size_t x,size_t y){
  struct position start = game_get_start(g);
  struct position obj;
  struct position hole;
    if (start.x == x && start.y == y) {
        printf(" > ");
        return 1;
    }
    for (size_t i = 0; i < game_get_n_objectives(g); i++) {
        obj = game_get_ith_objective(g,i);
        if (obj.x == x && obj.y == y) {
            printf(" $ ");
            return 2;
        }
    }
    for (size_t i = 0; i < game_get_n_holes(g); i++) {
        hole = game_get_ith_hole(g,i);
        if (hole.x == x && hole.y == y) {
            printf(" $ ");
            return 3;
        }
    }
    return 0;
}


void server_print_tab(game_t g){
    printf("server_print_tab\n");
    int dir;
    for (size_t i = 0 ; i < game_get_width(g) ; i++) {
        for (size_t j = 0; j < game_get_height(g) ; j++) {
            dir = game_get_dir(g,i,j);
            if (!print_positions(g, i, j)) {

                switch ( game_get_card(g,i,j) ) {
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
}

int initialize_gold_tab( void ){
    int n_cards =  NB_GOLD_CARD1 + NB_GOLD_CARD2 + NB_GOLD_CARD3;
    gold_cards_g.n_gold_cards = n_cards;
    for( int i=0; i < n_cards ; i++){
        gold_cards_g.gold_tab[i] = -1;
    }
    return 0;
}

int fill_gold_tab( void ){
    int n_cards = NB_GOLD_CARD1 + NB_GOLD_CARD2 + NB_GOLD_CARD3;
    int gold_cards_u[n_cards];
    for( unsigned int i = 0; i < NB_GOLD_CARD1; i++ )
        gold_cards_u[i] = 1;
    for( unsigned int i = 0; i < NB_GOLD_CARD2; i++ )
        gold_cards_u[NB_GOLD_CARD1 + i] = 2;
    for( unsigned int i = 0; i <NB_GOLD_CARD3; i++ )
        gold_cards_u[NB_GOLD_CARD1 + NB_GOLD_CARD2 + i] = 3;

    initialize_gold_tab();
    for ( int i=0; i < gold_cards_g.n_gold_cards ; i++){
        int indice_aleatoire = rand()%gold_cards_g.n_gold_cards;
        srand(time(NULL));
        while ( gold_cards_g.gold_tab[indice_aleatoire] != - 1){
            indice_aleatoire = (indice_aleatoire+1) % gold_cards_g.n_gold_cards;
        }
        gold_cards_g.gold_tab[indice_aleatoire] = gold_cards_u[i];
    }
    return 0;
}

int random_gold( void ){
    int gold = gold_cards_g.gold_tab[gold_cards_g.n_gold_cards];
    gold_cards_g.n_gold_cards = gold_cards_g.n_gold_cards - 1;
    return gold;
}



int position_comparison_end( game_t g, int previous_move_i, int objectives_j ){
    struct move prev_m= gameServer_get_ith_previous_move(g,previous_move_i);
    struct position obj = game_get_ith_objective(g,objectives_j);
    if ( (prev_m.onto.x ==obj.x) && (prev_m.onto.y == obj.y ) )
        return 1;
    return 0;
}


/**
 *
 * @param  g [description]
 * @return   return -1 if error
 *           the player id found the treasure
 */
int winner( game_t g){
    int w_player_id;
    size_t n_move = gameServer_get_n_moves(g);
    size_t n_play = game_get_n_players(g);
    for( unsigned int i=0; i < n_move; i++ ){
        for( unsigned int j=0; j < game_get_n_objectives(g); j++)
            if (position_comparison_end( g, i, j) ){
                return gameServer_get_ith_previous_move(g,i).player;
        }

    }
    return -1;
}


void gold_distribution (game_t g) {
    int gold;
    int n_saboteurs;
    int w_player_id = winner(g);
    size_t n_player = game_get_n_players(g);
    if (n_player >= 3 && n_player <= 4)
        n_saboteurs = 1;
    else if (n_player >= 5 && n_player <= 6)
        n_saboteurs = 2;
    else if (n_player >= 7 && n_player <= 9)
        n_saboteurs = 3;
    else
        n_saboteurs = 4;

    unsigned int n_gold_card = n_player;

    if( n_player == 10)
        n_gold_card = 9;

    if( gameServer_player_get_type(g, w_player_id) == SAPOTEUR ){
        if( n_saboteurs == 1 ){
            for(unsigned int i=0; i < n_player; i++)
                if( gameServer_player_get_type(g,i) == SAPOTEUR )
                    gameServer_player_set_gold(g,i,4);
        }
        if( n_saboteurs == 2 || n_saboteurs == 3 ){
            for(unsigned int i=0; i < n_player; i++)
                if( gameServer_player_get_type(g,i) == SAPOTEUR )
                gameServer_player_set_gold(g,i,3);
;
        }
        if( n_saboteurs == 4 ){
            for(unsigned int i=0; i < n_player; i++)
                if( gameServer_player_get_type(g,i) == SAPOTEUR )
                gameServer_player_set_gold(g,i,2);
;
        }
    }
    else{
        unsigned int j = w_player_id;
        for( unsigned int i=0; i < n_gold_card; i++ ){
            if( j > n_player)
                j = j%n_player;
            if( gameServer_player_get_type(g,i) == HONEST )
                gold = random_gold();
                gameServer_player_set_gold(g,i,gold);
            j++;
        }
    }
}
