/**
 * \file parser.c
 * \brief Contains the definitions of the functions used to parse a sapotache file
 * \author PARPAITE Thibault <br>
 * \date 06/03/2017
 */

/* XOPEN_SOURCE is required by strdup */
#define _XOPEN_SOURCE 500

#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "token_private.h"
#include "sapotache_interface.h"


/****************************************************
 * PRIVATE : TYPEDEF, STRUCTURES, STATICS FUNCTIONS *
 ****************************************************/

/* Used to check if ID matches a valid card */
#define N_MANDATORY_CARDS 8
#define N_OPTIONAL_CARDS 8

static const char *S_mandatory_cards_a[] = { "V_LINE", "H_LINE", "V_CROSS", "H_CROSS", "X_CROSS", "L_TURN", "R_TURN", "D_END" };
static const char *S_optional_cards_a[] = { "BOULDER", "B_AXE", "B_LAMP", "B_CART", "R_AXE", "R_LAMP", "R_CART", "R_ALL" };


struct memory_s {
    int width;
    int height;
    coord_t start;
    list_t objectives;
    list_t holes;
    int n_types_cards;
    int n_cards;
    int allow_boulder;
    int allow_breaks;
    int break_minus_repair;     /* repair == break if this value is equal to 0 */
    list_t l_deck;
};


static token_t token_create();
static int token_free(token_t t);
static memory_t memory_create();
static int memory_free(memory_t mem);
static int memory_add_objectives(memory_t mem, int x, int y);
static int memory_add_holes(memory_t mem, int x, int y);

static void parse_error(FILE *f, token_t t, memory_t mem, char *where, enum token_type type_expected, enum token_type type_read);
static void parse_error_id(FILE *f, token_t t, memory_t mem, char *where, const char *cards_expected_a[], int n);
static int parse_check_token(FILE *f, token_t t, memory_t mem, char *where, enum token_type type_expected);
static int parse_check_id(FILE *f, token_t t, memory_t mem, char *where, const char *cards_expected_a[], int n);
static int parse_dimensions(FILE *f, token_t t, memory_t mem);
static int parse_board(FILE *f, token_t t, memory_t mem);
static int parse_cards(FILE *f, token_t t, memory_t mem);

static enum card_id str_to_card_id(char *str);
static char *tokentype_to_str(int tokentype);
static int print_mem(memory_t mem);

static enum card_id *card_copy(enum card_id *c);
static int card_free(enum card_id *c);
static int card_print(enum card_id *c);


/*******************
 * TOKEN & MEMORY  *
 *******************/

static token_t token_create() {
    token_t t = malloc(sizeof (struct token_s));
    t->type = SPACES;
    t->str[0] = '\0';

    return t;
}


static int token_free(token_t t) {
    free(t);
    return 0;
}


static memory_t memory_create() {
    memory_t mem = malloc(sizeof (struct memory_s));
    mem->width = 0;
    mem->height = 0;
    mem->objectives = list_create((copy_function_t) coord_copy,
                                  (free_function_t) coord_free,
                                  (print_function_t) coord_print);
    mem->holes      = list_create((copy_function_t) coord_copy,
                                  (free_function_t) coord_free,
                                  (print_function_t) coord_print);
    mem->n_types_cards = 0;
    mem->n_cards = 0;
    mem->allow_boulder = 0;
    mem->allow_breaks = 0;
    mem->break_minus_repair = 0;
    mem->l_deck      = list_create((copy_function_t) card_copy,
                                   (free_function_t) card_free,
                                   (print_function_t) card_print);

    return mem;
}


static int memory_free(memory_t mem) {
    list_free(mem->objectives);
    list_free(mem->holes);
    list_free(mem->l_deck);
    free(mem);

    return 0;
}


static int memory_add_objectives(memory_t mem, int x, int y) {
    coord_t c = coord_create(x, y);
    list_add_tail(mem->objectives, (void *) c);
    free(c);

    return 0;
}


static int memory_add_holes(memory_t mem, int x, int y) {
    coord_t c = coord_create(x, y);
    list_add_tail(mem->holes, (void *) c);
    free(c);

    return 0;
}


static enum card_id *card_copy(enum card_id *c) {
    enum card_id *res = malloc(sizeof (enum card_id));
    *res = *c;
    return res;
}


static int card_free(enum card_id *c) {
    free(c);
    return 1;
}


static int card_print(enum card_id *c) {
    printf("%d", *c);
    return 1;
}


/*************************
 * OTHER : used to debug *
 *************************/

static char *tokentype_to_str(int tokentype) {
    char *str;

    switch(tokentype) {
    case ID:
        str = strdup("ID");
        break;
    case NUMBER:
        str = strdup("NUMBER");
        break;
    case STAR:
        str = strdup("STAR");
        break;
    case ARROW:
        str = strdup("ARROW");
        break;
    case DOLLAR:
        str = strdup("DOLLAR");
        break;
    case PERCENT:
        str = strdup("PERCENT");
        break;
    case NEW_LINE:
        str = strdup("NEW_LINE");
        break;
    case SPACES:
        str = strdup("SPACES");
        break;
    case LINE_CONTENT:
        str = strdup("LINE_CONTENT");
        break;
    case END_OF_FILE:
        str = strdup("END_OF_FILE");
        break;
    default:
        perror("*** tokentype_to_str : token not found");
        exit(1);
        break;
    }

    return str;
}


static enum card_id str_to_card_id(char *str) {
    if (!strcmp(str, "V_LINE"))
        return CARD_V_LINE;

    if (!strcmp(str, "H_LINE"))
        return CARD_H_LINE;

    if (!strcmp(str, "V_CROSS"))
        return CARD_V_CROSS;

    if (!strcmp(str, "H_CROSS"))
        return CARD_H_CROSS;

    if (!strcmp(str, "X_CROSS"))
        return CARD_X_CROSS;

    if (!strcmp(str, "L_TURN"))
        return CARD_L_TURN;

    if (!strcmp(str, "R_TURN"))
        return CARD_R_TURN;

    if (!strcmp(str, "D_END"))
        return CARD_D_END;

    if (!strcmp(str, "BOULDER"))
        return CARD_BOULDER;

    if (!strcmp(str, "B_AXE"))
        return CARD_B_AXE;

    if (!strcmp(str, "B_LAMP"))
        return CARD_B_LAMP;

    if (!strcmp(str, "B_CART"))
        return CARD_B_CART;

    if (!strcmp(str, "R_AXE"))
        return CARD_R_AXE;

    if (!strcmp(str, "R_LAMP"))
        return CARD_R_LAMP;

    if (!strcmp(str, "R_CART"))
        return CARD_R_CART;

    if (!strcmp(str, "R_ALL"))
        return CARD_R_ALL;

    return NO_CARD;
}


static int print_mem(memory_t mem) {
    printf("Configuration : %dx%d\n", mem->height, mem->width);
    printf("Card types    : %d\n", 16);
    printf("Nb of cards   : %d\n", mem->n_cards);
    printf("Objectives    : %zu ", list_size(mem->objectives)); list_print(mem->objectives); printf("\n");
    printf("Holes         : %zu ", list_size(mem->holes)); list_print(mem->holes); printf("\n");
    printf("Allow boulder : %s\n", mem->allow_boulder?"yes":"no");
    printf("Allow breaks  : %s\n", mem->allow_breaks?"yes":"no");
    printf("Repair=Break  : %s\n", (mem->break_minus_repair == 0)?"yes":"no");
    return 0;
}


/***********
 * PARSING *
 ***********/

static void parse_error(FILE *f, token_t t, memory_t mem, char *where, enum token_type type_expected, enum token_type type_read) {
    char *str_type_expected = tokentype_to_str(type_expected);
    char *str_type_read = tokentype_to_str(type_read);

    fprintf(stderr, "*** Syntax error : %s\n", where);
    fprintf(stderr, "Token expected : %s, but got token of type %s\n", str_type_expected, str_type_read);
    fprintf(stderr, "Read : %str", t->str);

    fclose(f);
    token_free(t);
    memory_free(mem);
    free(str_type_expected);
    free(str_type_read);

    exit(1);
}


static void parse_error_id(FILE *f, token_t t, memory_t mem, char *where, const char *cards_expected_a[], int n) {
    fprintf(stderr, "*** Syntax error : %s\n", where);
    fprintf(stderr, "ID str expected : ");

    for(int i = 0 ; i < n ; ++i)
        fprintf(stderr, "%s ", cards_expected_a[i]);

    fprintf(stderr, "\nBut got ID str %s\n", t->str);

    fclose(f);
    token_free(t);
    memory_free(mem);

    exit(1);
}


static int parse_check_token(FILE *f, token_t t, memory_t mem, char *where, enum token_type type_expected) {
    /* Reading the next token on the stream */
    lexer(f, t);

    /* Checking if it's syntaxically correct */
    if (t->type != type_expected)
        parse_error(f, t, mem, where, type_expected, t->type);

    return 0;
}


static int parse_check_id(FILE *f, token_t t, memory_t mem, char *where, const char *cards_expected_a[], int n) {
    /* Checking if token str ID matches the card expected */
    for (int i = 0 ; i < n ; ++i) {
        if (!strcmp(t->str, cards_expected_a[i]))
            return 0;
    }

    parse_error_id(f, t, mem, where, cards_expected_a, n);
    return 1;
}


/* parse_dimensions
 * DIM : NUMBER SPACES NUMBER NEW_LINE */
static int parse_dimensions(FILE *f, token_t t, memory_t mem) {
    parse_check_token(f, t, mem, "dimensions", NUMBER);
    mem->width = atoi(t->str);

    parse_check_token(f, t, mem, "dimensions", SPACES);

    parse_check_token(f, t, mem, "dimensions", NUMBER);
    mem->height = atoi(t->str);

    parse_check_token(f, t, mem, "dimensions", NEW_LINE);

    return 0;
    /* Dimensions successfuly read */
}


/* parse_board */
static int parse_board(FILE *f, token_t t, memory_t mem) {
    /* Dimensions were read, we can browse the board */

    for (int i = 0 ; i < mem->height ; ++i) {
        for (int j = 0 ; j < mem->width ; ++j) {
            /* Reading the next token on the stream */
            lexer(f, t);

            /* Checking error */
            if (t->type != STAR && t->type != ARROW && t->type != DOLLAR && t->type != PERCENT)
                parse_error(f, t, mem, "board", LINE_CONTENT, t->type);

            /* Action */
            switch(t->type) {
            case DOLLAR:
                memory_add_objectives(mem, j, mem->height - 1 - i);
                break;
            case PERCENT:
                memory_add_holes(mem, j, mem->height - 1 - i);
                break;
            case ARROW:
                mem->start = coord_create(j, mem->height - 1 - i);
                break;
            case STAR:
            default:
                break;
            }
        }

        /* NEW_LINE */
        parse_check_token(f, t, mem, "board", NEW_LINE);

        /* Board successfuly read */
    }

    return 0;
}



/* parse_cards *
 * CARD : ID SPACE NUMBER NEW_LINE */
static int parse_cards(FILE *f, token_t t, memory_t mem) {
    int n_current_card;
    enum card_id current_card;

    /* Browsing the 8 mandatory cards */
    for (int i = 0 ; i < 8 ; ++i) {
        parse_check_token(f, t, mem, "cards", ID);
        parse_check_id(f, t, mem, "cards", S_mandatory_cards_a, N_MANDATORY_CARDS);

        current_card = str_to_card_id(t->str);

        parse_check_token(f, t, mem, "cards", SPACES);

        parse_check_token(f, t, mem, "cards", NUMBER);
        mem->n_types_cards++;
        n_current_card = atoi(t->str);
        mem->n_cards += n_current_card;

        /* Adding the current card n times to the deck */
        for (int j = 0 ; j < n_current_card ; ++j)
            list_add_head(mem->l_deck, (void *) &current_card);

        parse_check_token(f, t, mem, "cards", NEW_LINE);
    }

    /* Optional cards */
    char ID_str[TOKEN_SIZE];

    /* Reading the next token on the stream (in case of EOF) */
    lexer(f, t);

    while (t->type != END_OF_FILE) {
        /* Checking error */
        if (t->type != ID)
            parse_error(f, t, mem, "cards", ID, t->type);

        parse_check_id(f, t, mem, "cards", S_optional_cards_a, N_OPTIONAL_CARDS);
        current_card = str_to_card_id(t->str);

        strcpy(ID_str, t->str);

        parse_check_token(f, t, mem, "cards", SPACES);

        parse_check_token(f, t, mem, "cards", NUMBER);

        mem->n_types_cards++;
        n_current_card = atoi(t->str);
        mem->n_cards += n_current_card;

        /* Adding the current card n times to the deck */
        for (int j = 0 ; j < n_current_card ; ++j)
            list_add_head(mem->l_deck, (void *) &current_card);

        parse_check_token(f, t, mem, "cards", NEW_LINE);

        /* Repairs == break */
        /* BOULDER CASE */
        if (!strcmp(ID_str, "BOULDER") && n_current_card > 0)
            mem->allow_boulder = 1;

        /* BREAK CASE */
        if (!strcmp(ID_str, "B_AXE") || !strcmp(ID_str, "B_LAMP") || !strcmp(ID_str, "B_CART")) {
            mem->allow_breaks = mem->allow_breaks || n_current_card;
            mem->break_minus_repair += n_current_card;
        }

        /* REPAIR CASE */
        if (!strcmp(ID_str, "R_AXE") || !strcmp(ID_str, "R_LAMP") ||
            !strcmp(ID_str, "R_CART") || !strcmp(ID_str, "R_ALL")) {
            mem->break_minus_repair -= n_current_card;
        }

        /* Reading the next token on the stream (in case of EOF) */
        lexer(f, t);
    }

    return 0;
    /* Cards successfuly read, EOF reached */
}


/********************
 * PUBLIC FUNCTIONS *
 ********************/

/* Parse a Sapotache file
 * print informations about file parsed
 * PRE: f has the expected structure */
int parse(FILE *f) {
    /* Allocation of token and memory */
    token_t t = token_create();
    memory_t mem = memory_create();

    /* start : dim board cards EOF */
    parse_dimensions(f, t, mem);
    parse_board(f, t, mem);
    parse_cards(f, t, mem);

    print_mem(mem);

    /* Free token and memory */
    token_free(t);
    memory_free(mem);

    return 0;
}


memory_t parse_memory(FILE *f) {
    /* Allocation of token and memory */
    token_t t = token_create();
    memory_t mem = memory_create();

    /* start : dim board cards EOF */
    parse_dimensions(f, t, mem);
    parse_board(f, t, mem);
    parse_cards(f, t, mem);

    /* Free token and memory */
    token_free(t);

    return mem;
}


int memory_get_width(memory_t mem) {
    return mem->width;
}


int memory_get_height(memory_t mem) {
    return mem->height;
}


coord_t memory_get_start(memory_t mem) {
    return mem->start;
}


unsigned int memory_get_n_cards(memory_t mem) {
    return mem->n_cards;
}


size_t memory_get_n_objectives(memory_t mem) {
    return list_size(mem->objectives);
}


list_t memory_get_objectives(memory_t mem) {
    return mem->objectives;
}


size_t memory_get_n_holes(memory_t mem) {
    return list_size(mem->holes);
}


list_t memory_get_holes(memory_t mem) {
    return mem->holes;
}


list_t memory_get_l_deck(memory_t mem) {
    return mem->l_deck;
}
