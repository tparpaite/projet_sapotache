/**
 * \file token.h
 * \brief Contains the declaration of common enum and structures for lexer and parser
 * \brief
 * \author PARPAITE Thibault <br>
 * \author SARRABAYROUSE Alexis <br>
 * \date 06/03/2017
 */

#ifndef _TOKEN_H
#define _TOKEN_H


#include <stdlib.h>


/* Definition of tokens constructed by the lexer */
enum token_type {
    ID,                    // [A-Za-z_]+
    NUMBER,                // [0-9]+
    STAR,                  // \*
    ARROW,                 // >
    DOLLAR,                // $
    PERCENT,               // \%
    NEW_LINE,              // \n
    SPACES,                // [[:space:]]+
    LINE_CONTENT,          // Used to display error, STAR | ARROW | DOLLAR | PERCENT
    END_OF_FILE,           // EOF
};


struct token_s;
typedef struct token_s *token_t;


#endif /* _TOKEN_H */
