#include <stdio.h>
#include "lexer.h"
#include "token_private.h"


/* Lexer
 * POST: t contains the next token from stream f.
 * The token value (str) is truncated at TOKEN_SIZE-1 bytes.
 */
int lexer(FILE *f, token_t t) {
    int already_consum = 0;
    char c = (char) fgetc(f);

    while (c != EOF) {
        switch (c) {
        case '*':
            t->type = STAR;
            return 1;
        case '>':
            t->type = ARROW;
            return 1;
        case '$':
            t->type = DOLLAR;
            return 1;
        case '%':
            t->type = PERCENT;
            return 1;
        case '\r':
        case '\n':
            // Ignore empty lines
            if (t->type != NEW_LINE) {
                t->type = NEW_LINE;
                return 1;
            }
            break;
        case ' ':
        case '\t':
            while (c == ' ' || c == '\t') {
                c = (char) fgetc(f);
            }
            // Ignore empty lines
            if (t->type != NEW_LINE) {
                ungetc((int) c, f);
                t->type = SPACES;
                return 1;
            }
            if (c != '\n' || c != '\r') {
                already_consum = 1;
            } else {
                t->type = NEW_LINE;
            }
            break;
        case '#':
            // Ignore comentary lines
            while (c != '\n' && c != '\r') {
                c = (char) fgetc(f);
            }
            // Change type by NEW_LINE
            // To know if we are at line beginning
            // when we read a space character
            t->type = NEW_LINE;
            if(c == EOF)
                already_consum = 1;
            break;

        default:
            if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')){
                int i = 0;
                t->type = ID;
                while (i < TOKEN_SIZE-1 && c != EOF && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'))) {
                    t->str[i] = c;
                    i++;
                    c = (char) fgetc(f);
                }
                t->str[i] = '\0';
                ungetc((int) c, f);
                return 1;
            }
            if(c >= '0' && c <= '9'){
                int i = 0;
                t->type = NUMBER;
                while (i < TOKEN_SIZE-1 && c >= '0' && c <= '9' && c != EOF) {
                    t->str[i] = c;
                    i++;
                    c = (char) fgetc(f);
                }
                t->str[i] = '\0';
                ungetc((int) c, f);
                return 1;
            }
            int i = 0;
            fprintf(stderr, "ERROR LEXER - %c : unknown character\n", c);
            t->type = LINE_CONTENT;
            while(i < TOKEN_SIZE-1 && c != EOF){
                t->str[i] = c;
                i++;
                c = (char) fgetc(f);
            }
            t->str[i] = '\0';
            ungetc((int) c, f);
            return 0;

        }
        if(!already_consum)
            c = (char) fgetc(f);
        already_consum = 0;
    }
    t->type = END_OF_FILE;
    return 1;
}
