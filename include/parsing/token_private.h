/**
 * \file token.h
 * \brief This file is supposed to be a private header
 * \brief Contains the definition of struct token_s
 * \author PARPAITE Thibault <br>
 * \author SARRABAYROUSE Alexis <br>
 * \date 06/03/2017
 */

#ifndef _TOKEN_PRIVATE_H
#define _TOKEN_PRIVATE_H


#define TOKEN_SIZE 16 /* The name of an ID has a max length of 16 */

struct token_s {
    enum token_type type;
    char str[TOKEN_SIZE];
};


#endif /* _TOKEN_PRIVATE_H */
