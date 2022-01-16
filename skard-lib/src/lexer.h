#ifndef SKARD_LEXER_H
#define SKARD_LEXER_H

#include <stdlib.h>

typedef enum {
    TOKEN_EOF, TOKEN_EOL, TOKEN_ERROR,

    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, // ( )
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, // { }
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET, // [ ]

    TOKEN_DOT, TOKEN_COMMA, TOKEN_COLON, // . , :

    TOKEN_PLUS, TOKEN_MINUS, TOKEN_RIGHT_ARROW, // + - ->
    TOKEN_STAR, TOKEN_SLASH, // * /
    TOKEN_AT,

    TOKEN_NOT, TOKEN_NOT_EQUAL, // ! !=
    TOKEN_ASSIGN, TOKEN_EQUAL, // = ==
    TOKEN_GREATER, TOKEN_GREATER_EQUAL, // > >=
    TOKEN_LESS, TOKEN_LESS_EQUAL, // < <=
    TOKEN_DIV, TOKEN_PIPE, TOKEN_OR, TOKEN_AND, // | |> || &&

    TOKEN_KEY_PACKAGE, TOKEN_KEY_IMPORT, // package, import
    TOKEN_KEY_STRUCT, TOKEN_KEY_SELF, // struct, self
    TOKEN_KEY_LET, TOKEN_KEY_NIL, // let, nil
    TOKEN_KEY_FN, TOKEN_KEY_RETURN, // fn, return
    TOKEN_KEY_IF, TOKEN_KEY_ELSE, // if, else
    TOKEN_KEY_WHILE, TOKEN_KEY_FOR, // while, for
    TOKEN_KEY_TRUE, TOKEN_KEY_FALSE, // true, false
    TOKEN_KEY_MATCH, TOKEN_KEY_WITH, // match, with
    TOKEN_KEY_DUMP, // dump

    TOKEN_IDENTIFIER, TOKEN_LIT_STRING, TOKEN_LIT_REAL, TOKEN_LIT_INT,

    TOKEN_COUNT
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
    size_t line;
    size_t column;
} Token;

typedef struct {
    const char *start;
    const char *current;
    size_t line;
    size_t column;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);

Token lexer_scan_token(Lexer *lexer);

#endif //SKARD_LEXER_H
