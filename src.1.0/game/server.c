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


#define NB_GOLD_CARD1 16
#define NB_GOLD_CARD2 8
#define NB_GOLD_CARD3 4

//int gold_cards[NB_GOLD_CARD1 + NB_GOLD_CARD2 +  NB_GOLD_CARD3];

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

typedef struct move (*client_play_t)(struct move const previous_moves[],
                                     size_t n_moves);
typedef int (*client_draw_card_t)(enum card_id card);

struct tile_s {
    enum card_id card;
    enum direction dir;
    int N;
    int S;
    int W;
    int E;
};


struct board_s {
    struct tile_s **tab;
    unsigned int width;
    unsigned int height;
    struct position start;
    size_t n_objectives;
    struct position *objectives;
    size_t n_holes;
    struct position *holes;
};

typedef struct board_s *board_t;


struct gold_cards{
    int n_gold_cards;
    int gold_tab[NB_GOLD_CARD1 + NB_GOLD_CARD2 + NB_GOLD_CARD3];
};

struct gold_cards gold_cards_g;


struct player_s {
    int id;
    void *handler;
    client_play_t play;
    client_draw_card_t draw_card;
    enum p_type type;
    size_t n_player_cards;
    enum card_id *player_cards;
    int kicked;
    int axe_broken;
    int lamp_broken;
    int cart_broken;
    int gold;
};

typedef struct player_s *player_t;


struct game_s {
    board_t b;
    unsigned int n_cards_total;
    list_t deck;
    unsigned int n_players;
    size_t n_moves;
    player_t *players_a;
    struct move *previous_moves;
};

typedef struct game_s *game_t;


/***** GAME / INITIALIZATION *****/
/**
 * @brief Create a new_game and initialize it
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


/***** BOARD *****/
/**
 * @brief Creates a new board from the information given by a parsed file????
 * @param mem, the set of information deduced from the parsing of a file
 * @return b, the new board if it was succesfully created
 *         NULL otherwise
 *
 */
static board_t server_new_board(memory_t mem);

/**
 * @brief Free memory of a board
 * @param b, a board
 * @return 0
 *
 */
static int server_free_board(board_t b);


/***** PLAYER *****/
/**
 * @brief Creates a new player from a client (a library)
 * @param client_library_path, the path to the client's library
 * @param id, an integer indicating whether the new client will be a sapoteur
 * @param type, the type of the new player
 * @param n_player_cards, the number of cards of the new player
 * @param player_cards, the array containing the cards of the new player
 * @return p, the new player
 *
 */
static player_t server_new_player(char *client_library_path, int id, enum p_type type, size_t n_player_cards, enum card_id *player_cards);

/**
 * @brief Free memory of a player
 * @param p, a player
 * @return 0
 *
 */
static int server_free_player(player_t p);

/**
 * @brief Adds a card to p's hand of cards
 * @param p, a player
 * @param card, the card to add
 * @return 0
 *
 */
static int server_add_player_card(player_t p, enum card_id card);

/**
 * @brief Removes a card from p's hand of cards
 * @param p, a player
 * @param card, the card to remove
 * @return 0 if it was correctly removed
 *         1 if the card was not found in his hand
 *
 */
static int server_remove_player_card(player_t p, enum card_id card);

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
static int server_update_cardinalpoints(struct tile_s *t);
static int server_is_start(game_t g, int x, int y);

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
static struct player_s *winner( game_t g);

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




static game_t server_new_game(char *board_path, int n_players, char **client_library_paths_a) {
    srand(time(NULL));

    /* Opening stream */
    FILE *f = fopen(board_path, "r");
    if (f == NULL) {
        fprintf(stderr, "*** %s : cannot open file %s\n", "server_new_game", board_path);
        exit(1);
    }

    /* Creating game */
    game_t g = malloc(sizeof (struct game_s));
    memory_t mem = parse_memory(f);
    g->b = server_new_board(mem);
    g->n_cards_total = memory_get_n_cards(mem);
    g->deck = list_copy(memory_get_l_deck(mem));
    list_shuffle(g->deck);
    fill_gold_tab();

    /* Creation of players */
    g->n_players = n_players;
    g->players_a = malloc(g->n_players * sizeof (player_t));

    /* Initializing previous moves */
    g->n_moves = 0;
    g->previous_moves = malloc(g->n_players * sizeof (struct move));

    int saboteurs[n_players];
    server_generate_saboteurs(g->n_players, saboteurs);
    int n_player_cards = server_n_player_cards(g->n_players);
    enum card_id *player_cards;

    for (unsigned int i = 0 ; i < g->n_players ; ++i) {
        player_cards = server_deal_initial_cards(g);
        g->players_a[i] = server_new_player(client_library_paths_a[i], i, saboteurs[i], n_player_cards, player_cards);

        g->players_a[i]->gold = 0; //TODO = initialiser le gold uniquement tous les n rounds.
        client_initialize_t c_initialize = dlsym(g->players_a[i]->handler, "initialize");
        /* This triggers a warning "ISO C forbids initialization between function pointer and ‘void *’".
           It can't be avoided because it is an old function and so it returns a void *... */
        if (c_initialize == NULL) {
            perror("Error loading c_initialize");
            exit(1);
        }

        c_initialize(g->players_a[i]->id,
                     g->players_a[i]->type,
                     g->b->width,
                     g->b->height,
                     g->b->start,
                     g->b->n_objectives,
                     g->b->objectives,
                     g->b->n_holes,
                     g->b->holes,
                     g->n_cards_total,
                     g->players_a[i]->n_player_cards,
                     g->players_a[i]->player_cards,
                     g->n_players);
    }

    return g;
}


static int server_free_game(game_t g) {
    server_free_board(g->b);
    list_free(g->deck);

    /* Free players */
    for (unsigned int i = 0 ; i < g->n_players ; ++i)
        server_free_player(g->players_a[i]);

    free(g->players_a);
    free(g->previous_moves);
    free(g);

    return 0;
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


static board_t server_new_board(memory_t mem) {
    board_t b = malloc(sizeof (struct board_s));
    b->width = memory_get_width(mem);
    b->height = memory_get_height(mem);
    b->start = server_coord_to_position(memory_get_start(mem));
    b->n_objectives = memory_get_n_objectives(mem);
    b->objectives = server_list_to_array(memory_get_objectives(mem));
    b->n_holes = memory_get_n_holes(mem);
    b->holes = server_list_to_array(memory_get_holes(mem));

    b->tab = malloc(sizeof (*(b->tab)) * b->width);
    if(b->tab == NULL){
        fprintf(stderr, "Error:%s:%d malloc Error\n", __FILE__, __LINE__ );
        return NULL;
    }
    for (size_t i = 0 ; i < b->width ; i++) {
        b->tab[i] = malloc(sizeof (*(b->tab[i])) * b->height);
        if(b->tab[i] == NULL) {
            fprintf(stderr, "Error:%s:%d malloc Error(i = %lu)\n", __FILE__, __LINE__, i );
            return NULL;
        }
        for (size_t j = 0; j < b->height ; j++) {
            b->tab[i][j].card = NO_CARD;
            b->tab[i][j].dir  = NORMAL;
            server_update_cardinalpoints(&b->tab[i][j]);
        }
    }

    return b;
}


static int server_free_board(board_t b) {
    for (size_t i = 0; i < b->height; i++)
        free(b->tab[i]);

    free(b->tab);
    free(b->objectives);
    free(b->holes);
    free(b);
    return 0;
}


static player_t server_new_player(char *client_library_path, int id,
                                  enum p_type type, size_t n_player_cards,
                                  enum card_id *player_cards) {
    player_t p = malloc(sizeof (struct player_s));
    p->id = id;
    p->handler = dlmopen(LM_ID_NEWLM, client_library_path, RTLD_NOW);
    p->play = dlsym(p->handler, "play");
    p->draw_card = dlsym(p->handler, "draw_card");
    p->type = type;
    p->n_player_cards = n_player_cards;
    p->player_cards = player_cards;
    p->kicked = 0;
    p->axe_broken = 0;
    p->lamp_broken = 0;
    p->cart_broken = 0;

    return p;
}


static int server_free_player(player_t p) {
    dlclose(p->handler);
    free(p->player_cards);
    free(p);
    return 1;
}


static int server_remove_player_card(player_t p, enum card_id card) {
    for (size_t i = 0 ; i < p->n_player_cards ; ++i) {
        /* Removing the card */
        if (p->player_cards[i] == card) {
            /* Left shift */
            p->n_player_cards--;
            for (size_t j = i ; j < p->n_player_cards ; ++j)
                p->player_cards[j] = p->player_cards[j+1];

            return 0;
        }
    }

    perror("Card wasn't found");
    return 1;
}


/* PRE : p->player_cards is an array of size n_cards_total */
static int server_add_player_card(player_t p, enum card_id card) {
    if (card != NO_CARD) {
        p->player_cards[p->n_player_cards] = card;
        p->n_player_cards++;
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
    int n_player_cards = server_n_player_cards(g->n_players);
    enum card_id *player_cards = malloc(g->n_cards_total * sizeof(enum card_id));

    for (int i = 0 ; i < n_player_cards ; ++i) {
        player_cards[i] = server_pop_card(g);
    }

    return player_cards;
}


static enum card_id server_pop_card(game_t g) {
    enum card_id card;

    /* Pop */
    if (!list_isempty(g->deck)) {
        list_begin(g->deck);
        card = *(enum card_id *) list_getdata(g->deck);
        list_remove_head(g->deck);
    } else {
        card = NO_CARD;
    }

    return card;
}


static int server_draw_card(game_t g, int player_id) {
    enum card_id card = server_pop_card(g);

    /* Updating server */
    server_add_player_card(g->players_a[player_id], card);

    /* Updating client */
    g->players_a[player_id]->draw_card(card);

    return 0;
}


int server_start_game(char *board_path, int n_players, char **client_library_paths_a) {
    game_t g = server_new_game(board_path, n_players, client_library_paths_a);
    enum p_type winner = server_main_loop(g);

    if (winner == SAPOTEUR)
        printf("SAPOTEUR won\n");
    else
        printf("HONEST won\n");

    server_free_game(g);

    return 0;
}


static int server_update_previousmoves(game_t g, struct move m) {
    /* First turn */
    if (g->n_moves < g->n_players) {
        g->previous_moves[g->n_moves] = m;
        g->n_moves++;

        return 0;
    }

    /* After first turn */
    /* Left shift */
    for (unsigned int i = 0 ; i < g->n_players - 1 ; ++i)
        g->previous_moves[i] = g->previous_moves[i+1];

    g->previous_moves[g->n_moves-1] = m;

    return 0;
}


static int server_kick_player(game_t g, int player_id, char *s) {
    perror(s);
    g->players_a[player_id]->kicked = 1;

    /* Don't need to update previous moves (first turn) */
    if (g->n_moves < g->n_players)
        return 0;

    /* Removing the player from previous moves */
    g->n_moves--;
    for (unsigned int i = 0 ; i < g->n_moves ; ++i)
        g->previous_moves[i] = g->previous_moves[i+1];

    return 0;
}


/* Initialize the values of the fields N, S, E and W of a tile based
   on the kind of card it represents */
static int server_update_cardinalpoints(struct tile_s *t) {
    switch (t->card){
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
        if (t->dir == NORMAL) {
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
        if (t->dir == NORMAL) {
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
        if (t->dir == NORMAL) {
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
        if (t->dir == NORMAL) {
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
    return (g->b->start.x == x) && (g->b->start.y == y);
}


/* Returns 0 if the move can be executed : the given card can be placed
  at the given set of the board.
  Returns 1 otherwise. */
static int server_check_cardinalpoints(game_t g, struct move m) {
    /* Storing the cards situated below, beside
       and above the one we would like to play */
    struct tile_s above;
    struct tile_s below;
    struct tile_s left;
    struct tile_s right;

    struct tile_s sentinel;
    sentinel.card = NO_CARD;
    sentinel.dir = NORMAL;
    server_update_cardinalpoints(&sentinel);

    int x = m.onto.x;
    int y = m.onto.y;
    int h = g->b->height;
    int l = g->b->width;

    if (x > 0)
        left = g->b->tab[x - 1][y];
    else
        left = sentinel;

    if (x < h - 1)
        right = g->b->tab[x + 1][y];
    else
        right = sentinel;

    if (y > 0)
        below = g->b->tab[x][y - 1];
    else
        below = sentinel;

    if (y < l - 1)
        above = g->b->tab[x][y + 1];
    else
        above = sentinel;

    /* Checking if the card is on a path (adjacent to start or another path card) */
    if (above.card == NO_CARD && !server_is_start(g, x - 1, y) &&
        below.card == NO_CARD && !server_is_start(g, x + 1, y) &&
        left.card == NO_CARD && !server_is_start(g, x, y - 1) &&
        right.card == NO_CARD && !server_is_start(g, x, y + 1)) {
        return 1;
    }

    /* Checking cardinal points */
    switch (m.card) {
    case CARD_V_LINE:
        if (above.S == 1 && below.N == 1 &&
            (left.card == NO_CARD || left.E == 0) &&
            (right.card == NO_CARD || right.W == 0))
            return 0;
        break;
    case CARD_H_LINE:
        if ((above.card == NO_CARD || above.S == 0) &&
            (below.card == NO_CARD || below.N == 0) &&
            left.E == 1 && right.W == 1)
            return 0;
        break;
    case CARD_V_CROSS:
        if (m.dir == NORMAL && above.S == 1 && below.N == 1 && left.E == 1 &&
            (right.card == NO_CARD || right.W == 0))
            return 0;
        if (m.dir == REVERSED && above.S == 1 && below.N == 1 && right.W == 1 &&
            (left.card == NO_CARD || left.E == 0))
           return 0;
        break;
    case CARD_H_CROSS:
        if (m.dir == NORMAL && above.S == 1 && right.W == 1 && left.E == 1 &&
            (below.card == NO_CARD || below.N == 0))
            return 0;
        if (m.dir == REVERSED && below.N == 1 && right.W == 1 && left.E == 1 &&
            (above.card == NO_CARD || above.S == 0))
            return 0;
        break;
    case CARD_L_TURN:
        if (m.dir == NORMAL && below.N == 1 && left.E == 1 &&
            (above.card == NO_CARD || above.S == 0) &&
            (right.card == NO_CARD || right.W == 0))
            return 0;
        if (m.dir == REVERSED && above.S == 1 && right.W == 1 &&
            (below.card == NO_CARD || below.N == 0) &&
            (left.card == NO_CARD || left.E == 0))
            return 0;
        break;
    case CARD_R_TURN:
        if (m.dir == NORMAL && below.N == 1 && right.W == 1 &&
            (above.card == NO_CARD || above.S == 0) &&
            (left.card == NO_CARD || left.E == 0))
            return 0;
        if (m.dir == REVERSED && above.S == 1 && left.E == 1 &&
            (below.card = NO_CARD || below.N == 0) &&
            (right.card = NO_CARD || right.W == 0))
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
    for (size_t i = 0 ; i < g->b->n_objectives ; ++i) {
        if (g->b->objectives[i].x == m.onto.x &&
            g->b->objectives[i].y == m.onto.y)
            return 1;
    }


    /* Checking if we are not playing on a hole */
    for (size_t i = 0 ; i < g->b->n_holes ; ++i) {
        if (g->b->holes[i].x == m.onto.x &&
            g->b->holes[i].y == m.onto.y)
            return 1;
    }


    /* Checking if we are not playing on start position */
    if (g->b->start.x == m.onto.x && g->b->start.y == m.onto.y)
        return 1;

    return 0;
}


static int server_move_isnotvalid(game_t g, struct move m) {
    /* Checking if the player got the card in his hand */
    int found = 0;
    for (int i = 0 ; g->players_a[m.player]->n_player_cards ; ++i) {
        if (g->players_a[m.player]->player_cards[i] == m.card) {
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
        if (m.onto.x >= g->b->width || m.onto.y >= g->b->height) {
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
        if (g->b->tab[m.onto.x][m.onto.y].card != NO_CARD) {
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
        if (m.onplayer >= g->n_players) {
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

    /* Play */
    while (cpt >= 0) {
        for (unsigned int i = 0 ; i < g->n_players ; ++i) {
            if (g->players_a[i]->kicked == 1)
                continue;

            struct move current_move = g->players_a[i]->play(g->previous_moves, g->n_moves);
            print_move(current_move);

            if (!server_move_isnotvalid(g, current_move)) {
                /* Player is kicked in server_move_isnotvalid */
                continue;
            }

            /* Updating previous_moves (for other clients) */
            server_update_previousmoves(g, current_move);

            /* Update the server game with the action related to move */
            server_play_move(g, current_move);

            /* Removing the card to the player */
            server_remove_player_card(g->players_a[i], current_move.card);

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

    g->b->tab[m.onto.x][m.onto.y].card = NO_CARD;
    g->b->tab[m.onto.x][m.onto.y].dir = NORMAL;
    server_update_cardinalpoints(&g->b->tab[m.onto.x][m.onto.y]);

    return 0;
}


/* Play a path card */
static int server_play_pathcard(game_t g, struct move m) {
    assert(m.act == ADD_PATH_CARD);

    g->b->tab[m.onto.x][m.onto.y].card = m.card;
    g->b->tab[m.onto.x][m.onto.y].dir = m.dir;
    /* Don't forget to update cardinal points */
    server_update_cardinalpoints(&g->b->tab[m.onto.x][m.onto.y]);

    return 0;
}


static int server_play_breakcard(game_t g, struct move m) {
    assert(m.act == PLAY_BREAK_CARD);

    switch (m.card) {
    case CARD_B_AXE:
        g->players_a[m.onplayer]->axe_broken = 1;
        break;
    case CARD_B_LAMP:
        g->players_a[m.onplayer]->lamp_broken = 1;
        break;
    case CARD_B_CART:
        g->players_a[m.onplayer]->cart_broken = 1;
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
        g->players_a[m.onplayer]->axe_broken = 0;
        break;
    case CARD_R_LAMP:
        g->players_a[m.onplayer]->lamp_broken = 0;
        break;
    case CARD_R_CART:
        g->players_a[m.onplayer]->cart_broken = 0;
        break;
    case CARD_R_ALL:
        g->players_a[m.onplayer]->axe_broken = 0;
        g->players_a[m.onplayer]->lamp_broken = 0;
        g->players_a[m.onplayer]->cart_broken = 0;
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


static int print_positions(board_t b,size_t x,size_t y){
    if (b->start.x == x && b->start.y == y) {
        printf(" > ");
        return 1;
    }
    for (size_t i = 0; i < b->n_objectives; i++) {
        if (b->objectives[i].x == x && b->objectives[i].y == y) {
            printf(" $ ");
            return 2;
        }
    }
    for (size_t i = 0; i < b->n_holes; i++) {
        if (b->holes[i].x == x && b->holes[i].y == y) {
            printf(" $ ");
            return 3;
        }
    }
    return 0;
}


void server_print_tab(game_t g){
    printf("server_print_tab\n");
    for (size_t i = 0 ; i < g->b->width ; i++) {
        for (size_t j = 0; j < g->b->height ; j++) {
            if (!print_positions(g->b, i, j)) {
                switch (g->b->tab[i][j].card) {
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
                    if (g->b->tab[i][j].dir == REVERSED) {
                        printf(" |-");
                    }else {
                        printf("-| ");
                    }
                    break;
                case CARD_H_CROSS:
                    if (g->b->tab[i][j].dir == REVERSED) {
                        printf("-,-");
                    }else {
                        printf("-'-");
                    }
                    break;
                case CARD_X_CROSS:
                    printf("-|-");
                    break;
                case CARD_L_TURN:
                    if (g->b->tab[i][j].dir == REVERSED) {
                        printf(" '-");
                    }else {
                        printf("-, ");
                    }
                    break;
                case CARD_R_TURN:
                    if (g->b->tab[i][j].dir == REVERSED) {
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
    if ( (g->previous_moves[previous_move_i].onto.x == g->b->objectives[objectives_j].x) && (g->previous_moves[previous_move_i].onto.y == g->b->objectives[objectives_j].y ) )
        return 1;
    return 0;
}

struct player_s *winner( game_t g){
    struct player_s *w_player = malloc(sizeof(struct player_s));
    for( unsigned int i=0; i < g->n_moves; i++ ){
        for( unsigned int j=0; j < g->b->n_objectives; j++)
            if (position_comparison_end( g, i, j) ){
                while(g->players_a[i]->type != HONEST) /* if a sapoteur finds a treasure, the first honest after him takes the first nuggets and the sapoteurs lose */
                    i = (i+1) % g->n_moves;
                w_player->id = g->previous_moves[i].player;
                w_player->type = HONEST;
                return w_player;
        }

    }
    if ( list_isempty(g->deck) & (g->players_a[g->n_players]->n_player_cards == 0)){
        w_player->type = SAPOTEUR;
        return w_player;
    }
    return NULL;
}


void gold_distribution (game_t g) {
    int n_saboteurs;
    struct player_s *w_player = winner(g);

    if (g->n_players >= 3 && g->n_players <= 4)
        n_saboteurs = 1;
    else if (g->n_players >= 5 && g->n_players <= 6)
        n_saboteurs = 2;
    else if (g->n_players >= 7 && g->n_players <= 9)
        n_saboteurs = 3;
    else
        n_saboteurs = 4;

    unsigned int n_gold_card = g->n_players;

    if( g->n_players == 10)
        n_gold_card = 9;

    if( w_player->type == SAPOTEUR ){
        if( n_saboteurs == 1 ){
            for(unsigned int i=0; i < g->n_players; i++)
                if( g->players_a[i]->type == w_player->type )
                    g->players_a[i]->gold = 4;
        }
        if( n_saboteurs == 2 || n_saboteurs == 3 ){
            for(unsigned int i=0; i < g->n_players; i++)
                if( g->players_a[i]->type == w_player->type )
                    g->players_a[i]->gold = 3;
        }
        if( n_saboteurs == 4 ){
            for(unsigned int i=0; i < g->n_players; i++)
                if( g->players_a[i]->type == w_player->type )
                    g->players_a[i]->gold = 2;
        }
    }
    else{
        unsigned int j = w_player->id;
        for( unsigned int i=0; i < n_gold_card; i++ ){
            if( j > g->n_players)
                j = j%g->n_players;
            if( g->players_a[j]->type == w_player->type )
                g->players_a[i]->gold = random_gold();
            j++;
        }
    }
}
