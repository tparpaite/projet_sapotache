/**
 * \file lexer.h
 * \brief Contains the declaration of the functions used to lex a file
 * \author PARPAITE Thibault <br>
 * \author SARRABAYROUSE Alexis <br>
 * \date 06/03/2017
 */

#ifndef _LEXER_H
#define _LEXER_H

#include <stdio.h>
#include "token.h"

int lexer(FILE *f, token_t t);


#endif /* _LEXER_H */
