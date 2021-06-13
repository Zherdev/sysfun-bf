/*
 * See parser/include/syntax.h for details.
 *
 * Zherdev, 2021
 */

#include "syntax.h"

#include <stdlib.h>
#include <stdarg.h>

static int8_t
syn_magazine_init(struct syn_magazine *magazine)
{
    const int32_t default_magazine_size = 1024;
    magazine->size = default_magazine_size;
    magazine->pos = 0;
    magazine->buff = calloc(1024, sizeof(magazine->buff));
    if (!magazine->buff) {
        return -1;
    }
    magazine->buff[0] = SYN_START;

    return 0;
}

static void
syn_magazine_free(struct syn_magazine *magazine)
{
    if (!magazine) {
        return;
    }

    free(magazine->buff);
    magazine->buff = NULL;
}

int8_t
syn_parser_init(struct syn_parser *parser)
{
    if (!parser) {
        return -1;
    }

    int8_t err = syn_magazine_init(&parser->magazine);
    if (err) {
        return -1;
    }

    parser->need_next_lex = 1;

    return 0;
}

void
syn_parser_free(struct syn_parser *parser)
{
    if (!parser) {
        return;
    }

    syn_magazine_free(&parser->magazine);
}

static enum syn_state
syn_magazine_pop(struct syn_magazine *magazine)
{
    if (magazine->pos >= magazine->size) {
        return SYN_ERR;
    }
    return magazine->buff[magazine->pos--];
}

static int8_t
syn_magazine_push_one(struct syn_magazine *magazine, enum syn_state state)
{
    magazine->pos++;
    if (magazine->pos >= magazine->size) {
        int32_t new_size = magazine->size * 2;
        magazine->buff = realloc(magazine->buff, new_size * sizeof(*magazine->buff));
        if (!magazine->buff) {
            return -1;
        }
        magazine->size = new_size;
    }

    magazine->buff[magazine->pos] = state;

    return 0;
}

static int8_t
syn_magazine_push(struct syn_magazine *magazine, int32_t n, ...)
{
    va_list va;
    va_start(va, n);

    for (int32_t i = 0; i < n; i++) {
        enum syn_state to_push = va_arg(va, enum syn_state);

        int8_t err = syn_magazine_push_one(magazine, to_push);
        if (err) {
            return -1;
        }
    }

    va_end(va);

    return 0;
}

int8_t
syn_parser_process_lex(struct syn_parser *parser, enum lex_terminal lex)
{
    struct syn_magazine *magazine = &parser->magazine;
    enum syn_state cur_state = syn_magazine_pop(magazine);

    parser->need_next_lex = 0;

    switch (cur_state) {
        case SYN_START:
            if (lex_is_common_symbol(lex)
                    || lex == LEX_DELIM
                    || lex == LEX_CYC_START
                    || lex == LEX_COMMENT) {
                return syn_magazine_push(magazine, 2, SYN_START, SYN_FUNC);
            }
            if (lex == LEX_EOF) {
                return syn_magazine_push_one(magazine, SYN_END);
            }
            break;

        case SYN_FUNC:
            if (lex_is_common_symbol(lex)
                    || lex == LEX_DELIM
                    || lex == LEX_CYC_START
                    || lex == LEX_COMMENT) {
                return syn_magazine_push(magazine, 2, SYN_DELIM, SYN_FUNC_BODY);
            }
            break;

        case SYN_FUNC_BODY:
            if (lex_is_common_symbol(lex)
                    || lex == LEX_CYC_START) {
                return syn_magazine_push(magazine, 2, SYN_FUNC_BODY, SYN_CONSTRUCT);
            }
            if (lex == LEX_COMMENT) {
                return syn_magazine_push(magazine, 2, SYN_COMMENT, SYN_COMMENT_START);
            }
            if (lex == LEX_DELIM) {
                return 0;
            }
            break;

        case SYN_CONSTRUCT:
            if (lex_is_common_symbol(lex)) {
                return syn_magazine_push(magazine, 2, SYN_CONSTRUCT, SYN_SYMBOL);
            }
            if (lex == LEX_CYC_START) {
                return syn_magazine_push(magazine, 2, SYN_CONSTRUCT, SYN_CYC);
            }
            if (lex == LEX_DELIM
                    || lex == LEX_CYC_END
                    || lex == LEX_COMMENT) {
                return 0;
            }
            break;

        case SYN_SYMBOL:
            if (lex_is_common_symbol(lex)) {
                parser->need_next_lex = 1;
                return 0;
            }
            break;

        case SYN_CYC:
            if (lex == LEX_CYC_START) {
                return syn_magazine_push(magazine, 2, SYN_CYC_END, SYN_CYC_START);
            }
            break;

        case SYN_CYC_START:
            if (lex == LEX_CYC_START) {
                parser->need_next_lex = 1;
                return syn_magazine_push_one(magazine, SYN_CONSTRUCT);
            }
            break;

        case SYN_CYC_END:
            if (lex == LEX_CYC_END) {
                parser->need_next_lex = 1;
                return 0;
            }
            break;

        case SYN_COMMENT_START:
            if (lex == LEX_COMMENT) {
                parser->need_next_lex = 1;
                return 0;
            }
            break;

        case SYN_COMMENT:
            if (lex == LEX_DELIM) {
                return 0;
            }
            parser->need_next_lex = 1;
            return syn_magazine_push_one(magazine, SYN_COMMENT);
            break;

        case SYN_DELIM:
            if (lex == LEX_DELIM) {
                parser->need_next_lex = 1;
                return 0;
            }
            break;

        case SYN_END:
            parser->need_next_lex = 0;
            return 0;
            break;

        default:
            break;
    }

    syn_magazine_push_one(magazine, SYN_ERR);
    return -1;
}

enum syn_state
syn_parser_cur_state(struct syn_parser *parser)
{
    if (!parser) {
        return SYN_ERR;
    }

    struct syn_magazine *magazine = &parser->magazine;

    if (magazine->pos >= magazine->size) {
        return SYN_ERR;
    }

    return magazine->buff[magazine->pos];
}