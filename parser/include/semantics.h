/*
 * Zherdev, 2021
 */

#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "lex.h"
#include "syntax.h"

#include <stdint.h>
#include <stdio.h>

enum sem_node_type {
    SEM_ROOT,

    SEM_FUNC,

    SEM_CYC,
    SEM_CYC_START,
    SEM_CYC_BODY,
    SEM_CYC_END,

    SEM_ACTION_INC,
    SEM_ACTION_DEC,
    SEM_ACTION_LEFT,
    SEM_ACTION_RIGHT,
    SEM_ACTION_INPUT,
    SEM_ACTION_OUTPUT,
    SEM_ACTION_UP,
    SEM_ACTION_DOWN,
    SEM_ACTION_FUNC_CALL,
    SEM_ACTION_RETURN,
    SEM_ACTION_SYS_CALL
};

struct sem_node {
    enum   sem_node_type  type;
    struct sem_node      *root;
    struct sem_node      *leaves;
    int32_t               leaves_num;
    int32_t               leaves_max_num;
};

struct sem_analyzer {
    struct sem_node    tree;
    struct sem_node   *cur_node;
    struct lex_parser *lexer;
    struct syn_parser *syntaxer;
};

int8_t
sem_analyzer_init(
        struct sem_analyzer *res,
        struct lex_parser   *lexer,
        struct syn_parser   *syntaxer);

void
sem_analyzer_free(struct sem_analyzer *analyzer);

int8_t
sem_analyzer_process(struct sem_analyzer *analyzer);

int8_t
sem_analyzer_tree_print(struct sem_analyzer *analyzer);

int8_t
sem_analyzer_err_fprint(struct sem_analyzer *analyzer, FILE *file);

int8_t
sem_node_is_action(enum sem_node_type node_type);

#endif // SEMANTICS_H