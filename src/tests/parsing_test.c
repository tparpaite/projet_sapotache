#include "parser.h"
 
/* Usage function */
static void usage(char *execname) {
    fprintf(stderr, "Usage: %s <filename>\n", execname);
    fprintf(stderr, "   where <filename> is the file to parse\n");
}


/* Main function */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("*** %s : cannot open file %s\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }

    /* Parsing */
    parse(f);

    fclose(f);

    return EXIT_SUCCESS;
}
