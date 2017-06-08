/* For strdup */
#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "server.h"
 
/* Usage function */
static void usage(char *execname) {
    fprintf(stderr, "Usage: %s <board_path> <client_library_path_1> <client_library_path_2> <client_library_path_3> [... <client_library_path_n>]\n", execname);
    fprintf(stderr, "   where <board_path> is the board to parse\n");
    fprintf(stderr, "         <client_library_path_i> the path to strategy (.so) of the client number i\n");
}


/* Main function */
int main(int argc, char *argv[]) {
    if (argc < 4) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Allocation */
    char *board_path = strdup(argv[1]);
    int n_players = argc - 2;
    char **client_library_path_a = malloc(n_players * sizeof(char *));

    for (int i = 0 ; i < n_players ; ++i)
        client_library_path_a[i] = strdup(argv[i + 2]);
    

    /* Launching the game */
    server_start_game(board_path, n_players, client_library_path_a);

    
    /* Free */
    free(board_path);

    for (int i = 0 ; i < n_players ; ++i)
        free(client_library_path_a[i]);

    free(client_library_path_a);

    return EXIT_SUCCESS;
}
