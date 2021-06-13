/*
 * Zherdev, 2021
 */

#ifndef LEX_H
#define LEX_H

#include <stdint.h>

enum lex_terminal {
    // Brainfuck symbols
    LEX_INC,       // +
    LEX_DEC,       // -
    LEX_LEFT,      // <
    LEX_RIGHT,     // >
    LEX_CYC_START, // [
    LEX_CYC_END,   // ]
    LEX_INPUT,     // ,
    LEX_OUTPUT,    // .

    // Brainfunction symbols
    LEX_UP,        // ^
    LEX_DOWN,      // v
    LEX_FUNC_CALL, // :
    LEX_RETURN,    // ;

    // Systemf symbols
    LEX_SYS_CALL, // %

    // Util
    LEX_COMMENT, // ' ' space
    LEX_DELIM,   // \n
    LEX_EOF,
    LEX_ERR,
    LEX_UNK
};

struct lex_parser {
    int32_t fd;
    enum lex_terminal next;
    char parsed;
};

int8_t
lex_parser_init(struct lex_parser *parser, int32_t fd);

int8_t
lex_parser_read(struct lex_parser *parser);

int8_t
lex_is_common_symbol(enum lex_terminal lex);

#endif // LEX_H