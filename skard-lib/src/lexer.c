#include "lexer.h"

#include <stdbool.h>
#include <string.h>

void lexer_init(Lexer *lexer, const char *source)
{
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 0;
}

static Token make_token(Lexer *lexer, TokenType type);
static Token make_eof_token(Lexer *lexer);
static Token make_eol_token(Lexer *lexer);
static Token make_error_token(Lexer *lexer, const char *message);

static bool lexer_is_at_end_of_file(Lexer *lexer);
static bool lexer_is_at_end_of_line(Lexer *lexer);

static char lexer_advance(Lexer *lexer);
static char lexer_peek(Lexer *lexer);
static char lexer_peek_next(Lexer *lexer);
static bool lexer_match_next(Lexer *lexer, char expected);

static bool is_digit(char c);
static bool is_alpha(char c);
static bool is_insignificant_whitespace(char c);

static void lexer_skip_single_line_comment(Lexer *lexer);
static void lexer_skip_multi_line_comment(Lexer *lexer);
static void lexer_skip_insignificant(Lexer *lexer);

static TokenType lexer_check_keyword(Lexer *lexer, size_t start, const char *rest, TokenType type);
static TokenType lexer_determine_identifier_type(Lexer *lexer);

static Token lexer_scan_string(Lexer *lexer);
static Token lexer_scan_number(Lexer *lexer);
static Token lexer_scan_identifier(Lexer *lexer);


static Token make_token(Lexer *lexer, TokenType type)
{
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = lexer->current - lexer->start;
    token.line = lexer->line;
    token.column = lexer->column - token.length + 1;
    return token;
}

static Token make_eof_token(Lexer *lexer)
{
    lexer->column++;
    return make_token(lexer, TOKEN_EOF);
}

static Token make_eol_token(Lexer *lexer)
{
    lexer->line++;
    return make_token(lexer, TOKEN_EOL);
}

static Token make_error_token(Lexer *lexer, const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}


static bool lexer_is_at_end_of_file(Lexer *lexer)
{
    return *lexer->current == '\0';
}

static bool lexer_is_at_end_of_line(Lexer *lexer)
{
    return *lexer->current == '\n';
}


static char lexer_advance(Lexer *lexer)
{
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}

static char lexer_peek(Lexer *lexer)
{
    return *lexer->current;
}

static char lexer_peek_next(Lexer *lexer)
{
    if (lexer_is_at_end_of_file(lexer)) {
        return '\0';
    }

    return lexer->current[1];
}


static bool lexer_match_next(Lexer *lexer, char expected)
{
    if (lexer_is_at_end_of_file(lexer) || *lexer->current != expected) {
        return false;
    }

    lexer_advance(lexer);
    return true;
}


static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_insignificant_whitespace(char c)
{
    return c == ' ' || c == '\r' || c == '\t';
}


static void lexer_skip_single_line_comment(Lexer *lexer)
{
    while (lexer_peek(lexer) != '\n' && !lexer_is_at_end_of_file(lexer)) {
        lexer_advance(lexer);
    }
}

static void lexer_skip_multi_line_comment(Lexer *lexer)
{
    while (!lexer_is_at_end_of_file(lexer)) {
        char c = lexer_advance(lexer);
        if (c == '*' && lexer_peek(lexer) == '/') {
            lexer_advance(lexer);
            break;
        }
    }
}

static void lexer_skip_insignificant(Lexer *lexer)
{
    while (true) {
        char c = lexer_peek(lexer);
        if (!is_insignificant_whitespace(c)) {
            if (c == '/' && lexer_peek_next(lexer) == '/') {
                lexer_skip_single_line_comment(lexer);
            }

            if (c == '/' && lexer_peek_next(lexer) == '*') {
                lexer_skip_multi_line_comment(lexer);
            }

            return;
        }

        lexer_advance(lexer);
    }
}


static TokenType lexer_check_keyword(Lexer *lexer, size_t start, const char *rest, TokenType type)
{
    size_t length = strlen(rest);
    if ((size_t) (lexer->current - lexer->start) == start + length &&
        memcmp(lexer->start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType lexer_determine_identifier_type(Lexer *lexer)
{
    switch (lexer->start[0]) {
        case 'd':
            return lexer_check_keyword(lexer, 1, "ump", TOKEN_KEY_DUMP);
        case 'e':
            return lexer_check_keyword(lexer, 1, "lse", TOKEN_KEY_ELSE);
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'a':
                        return lexer_check_keyword(lexer, 2, "lse", TOKEN_KEY_FALSE);
                    case 'n':
                        return lexer_check_keyword(lexer, 2, "", TOKEN_KEY_FN);
                    case 'o':
                        return lexer_check_keyword(lexer, 2, "r", TOKEN_KEY_FOR);
                    default:
                        break;
                }
            }
            break;
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'f':
                        return lexer_check_keyword(lexer, 2, "", TOKEN_KEY_IF);
                    case 'm':
                        return lexer_check_keyword(lexer, 2, "port", TOKEN_KEY_IMPORT);
                    default:
                        break;
                }
            }
            break;
        case 'l':
            return lexer_check_keyword(lexer, 1, "et", TOKEN_KEY_LET);
        case 'm':
            return lexer_check_keyword(lexer, 1, "atch", TOKEN_KEY_MATCH);
        case 'n':
            return lexer_check_keyword(lexer, 1, "il", TOKEN_KEY_NIL);
        case 'p':
            return lexer_check_keyword(lexer, 1, "ackage", TOKEN_KEY_PACKAGE);
        case 'r':
            return lexer_check_keyword(lexer, 1, "eturn", TOKEN_KEY_RETURN);
        case 's':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'e':
                        return lexer_check_keyword(lexer, 2, "lf", TOKEN_KEY_SELF);
                    case 't':
                        return lexer_check_keyword(lexer, 2, "ruct", TOKEN_KEY_STRUCT);
                    default:
                        break;
                }
            }
            break;
        case 't':
            return lexer_check_keyword(lexer, 1, "rue", TOKEN_KEY_TRUE);
        case 'w':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->start[1]) {
                    case 'h':
                        return lexer_check_keyword(lexer, 2, "ile", TOKEN_KEY_WHILE);
                    case 'i':
                        return lexer_check_keyword(lexer, 2, "th", TOKEN_KEY_WITH);
                    default:
                        break;
                }
            }
        default:
            break;
    }

    return TOKEN_IDENTIFIER;
}


static Token lexer_scan_string(Lexer *lexer)
{
    while (lexer_peek(lexer) != '"' && !lexer_is_at_end_of_line(lexer) && lexer_is_at_end_of_file(lexer)) {
        lexer_advance(lexer);
    }

    if (lexer_is_at_end_of_file(lexer) || lexer_is_at_end_of_line(lexer)) {
        return make_error_token(lexer, "Unterminated string literal");
    }

    lexer_advance(lexer);
    return make_token(lexer, TOKEN_LIT_STRING);
}

static Token lexer_scan_number(Lexer *lexer)
{
    while (is_digit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }

    if (lexer_peek(lexer) == '.') {
        lexer_advance(lexer);
        while (is_digit(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }

        return make_token(lexer, TOKEN_LIT_REAL);
    }

    return make_token(lexer, TOKEN_LIT_INT);
}

static Token lexer_scan_identifier(Lexer *lexer)
{
    while (is_alpha(lexer_peek(lexer)) || is_digit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }

    return make_token(lexer, lexer_determine_identifier_type(lexer));
}


Token lexer_scan_token(Lexer *lexer)
{
    lexer_skip_insignificant(lexer);

    if (lexer_is_at_end_of_file(lexer)) {
        return make_eof_token(lexer);
    }

    if (lexer_is_at_end_of_line(lexer)) {
        lexer_advance(lexer);
        return make_eol_token(lexer);
    }

    char c = lexer_advance(lexer);

    if (is_alpha(c)) {
        return lexer_scan_identifier(lexer);
    }

    if (is_digit(c)) {
        return lexer_scan_number(lexer);
    }

    switch (c) {
        case '(':
            return make_token(lexer, TOKEN_LEFT_PAREN);
        case ')':
            return make_token(lexer, TOKEN_RIGHT_PAREN);
        case '{':
            return make_token(lexer, TOKEN_LEFT_BRACE);
        case '}':
            return make_token(lexer, TOKEN_RIGHT_BRACE);
        case '[':
            return make_token(lexer, TOKEN_LEFT_BRACKET);
        case ']':
            return make_token(lexer, TOKEN_RIGHT_BRACKET);
        case '.':
            return make_token(lexer, TOKEN_DOT);
        case ',':
            return make_token(lexer, TOKEN_COMMA);
        case ':':
            return make_token(lexer, TOKEN_COLON);
        case '+':
            return make_token(lexer, TOKEN_PLUS);
        case '-':
            return make_token(lexer, lexer_match_next(lexer, '>') ? TOKEN_RIGHT_ARROW : TOKEN_MINUS);
        case '*':
            return make_token(lexer, TOKEN_STAR);
        case '/':
            return make_token(lexer, TOKEN_SLASH);
        case '@':
            return make_token(lexer, TOKEN_AT);
        case '!':
            return make_token(lexer, lexer_match_next(lexer, '=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
        case '=':
            return make_token(lexer, lexer_match_next(lexer, '=') ? TOKEN_EQUAL : TOKEN_ASSIGN);
        case '>':
            return make_token(lexer, lexer_match_next(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '<':
            return make_token(lexer, lexer_match_next(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '|':
            return make_token(lexer, lexer_match_next(lexer, '>') ? TOKEN_PIPE :
                                     (lexer_match_next(lexer, '|') ? TOKEN_OR : TOKEN_DIV));
        case '&':
            return lexer_match_next(lexer, '&') ?
                   make_token(lexer, TOKEN_AND) : make_error_token(lexer, "Unexpected character");
        case '"':
            return lexer_scan_string(lexer);
        default:
            break;
    }

    return make_error_token(lexer, "Unexpected character");
}
