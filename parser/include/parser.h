/*
 * Zherdev, 2021
 */

#ifndef PARSER_H
#define PARSER_H

#include "lex.h"
#include "syntax.h"
#include "semantics.h"

#include <stdint.h>

struct parser {
    struct lex_parser   lexer;
    struct syn_parser   syntaxer;
    struct sem_analyzer analyzer;
    int32_t fd;
};

int8_t
parser_init(struct parser *parser, const char *filename);

int8_t
parser_free(struct parser *parser);

int8_t
parser_process_file(struct parser *parser);

int8_t
parser_err_to_stderr(struct parser *parser);

#endif // PARSER_H