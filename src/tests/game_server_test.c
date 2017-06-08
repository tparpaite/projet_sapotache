#include "../../include/game/game_server.h"
#include "../../include/ADT/adt_list.h"
#include "parser.h"

void *cpy_a(void *data){
    return data;
}
int   free_a(void *data){
    return 0;
}
int   print_a(void *data){
    return 0;
}

static struct position server_coord_to_position(coord_t c) {
    struct position p;
    p.x = coord_getX(c);
    p.y = coord_getY(c);
    return p;
}

static struct position *list_to_array(list_t l) {
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
int main(int argc, char const *argv[]) {

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr, "*** %s : cannot open file %s\n", "server_new_game", argv[1]);
        exit(1);
    }
    memory_t mem = parse_memory(f);

    game_t my_game = gameServer_new(
        memory_get_width(mem),
        memory_get_height(mem),
        memory_get_n_objectives(mem),
        list_to_array(memory_get_objectives(mem)),
        server_coord_to_position(memory_get_start(mem)),
        memory_get_n_holes(mem),
        list_to_array(memory_get_holes(mem)),
        memory_get_n_cards(mem),
        3,
        memory_get_l_deck(mem)

    );
    game_print_board(my_game);
    return 0;
}
