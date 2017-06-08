#include "../../include/game/gameClient.h"

// Pour les fonctions on transmet toujours le game_t
server_remove_player_card(g->players_a[i], current_move.card);
server_remove_player_card(g, current_move.card);

// toutes les foncitons prennent 
// les écriture se font avec des setter
// les fonctions on un nom du type game_set_le_champs(la_partie [, indice | x, y] , new_val)
//  Pour les fonctions specifiques au client c'est gameClient_...
//  Pour les fonctions specifiques au serveur c'est gameServer_...
my_game.type = type;
gameCleint_set_type(my_game, type);

my_game.n_player_cards = n_player_cards;
gameCleint_set_n_player_cards(my_game, n_player_cards);

my_game.board.tab[previous_moves[i].onto.x][previous_moves[i].onto.y].dir = previous_moves[i].dir;
game_set_dir(my_game, previous_moves[i].onto.x, previous_moves[i].onto.y, previous_moves[i].dir);


// les lecture avec des getter
// game_get_le_champs(game_t, [, indice | x, y])
my_move.player = my_game.id;
my_move.player = gameCleint_get_id(my_game);

my_move.onto.x = rand() % my_game.board.width;
my_move.onto.x = rand() % game_get_width(my_game);

my_game.player_cards[my_game.n_player_cards] = card;
gameCleint_set_ith_player_cards(my_game, gameCleint_get_n_player_cards(my_game), card);

my_game.n_player_cards++;
gameCleint_set_n_player_cards(my_game, gameCleint_get_n_player_cards(my_game)+1);


/////
///// Server
/////

for (unsigned int i = 0 ; i < g->n_players ; ++i) {
for (unsigned int i = 0 ; i < game_get_n_players(g) ; ++i) {

struct move current_move = g->players_a[i]->play(g->previous_moves, g->n_moves);
struct move current_move = gameServer_player_play(g, i, gameServer_get_previous_moves(g), gameServer_get_n_moves(g));
