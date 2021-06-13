/*
 * Zherdev, 2021
 */

#ifndef SYNTAX_H
#define SYNTAX_H

#include "lex.h"

#include <stdint.h>

enum syn_state {
    SYN_START,
    SYN_END,

    SYN_FUNC,
    SYN_FUNC_BODY,

    SYN_CONSTRUCT,

    SYN_SYMBOL,

    SYN_CYC,
    SYN_CYC_START,
    SYN_CYC_END,

    SYN_COMMENT,
    SYN_COMMENT_START,
    SYN_DELIM,
    SYN_ERR
};

struct syn_magazine {
    enum syn_state *buff;
    int32_t pos;
    int32_t size;
};

struct syn_parser {
    struct syn_magazine magazine;
    int8_t need_next_lex;
};

int8_t
syn_parser_init(struct syn_parser *parser);

void
syn_parser_free(struct syn_parser *parser);

int8_t
syn_parser_process_lex(struct syn_parser *parser, enum lex_terminal lex);

enum syn_state
syn_parser_cur_state(struct syn_parser *parser);

#endif // SYNTAX_H