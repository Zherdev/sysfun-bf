/*
 * See parser/include/semantics.h for details.
 *
 * Zherdev, 2021
 */

#include "semantics.h"

#include <stdlib.h>
#include <stdio.h>

static void
sem_node_tree_free(struct sem_node *tree)
{
    if (!tree) {
        return;
    }

    for (int32_t i = 0; i < tree->leaves_num; i++) {
        sem_node_tree_free(&tree->leaves[i]);
    }

    free(tree->leaves);
}

static void
sem_node_init(struct sem_node *node)
{
    node->leaves = NULL;
    node->leaves = NULL;
    node->leaves_max_num = 0;
    node->leaves_num = 0;
    node->type = 0;
}

static int32_t
sem_node_alloc_leaves(struct sem_node *node)
{
    const int32_t default_leaves_num = 32;

    node->leaves = calloc(default_leaves_num, sizeof(*node->leaves));
    if (!node->leaves) {
        return -1;
    }

    node->leaves_max_num = default_leaves_num;

    return 0;
}

static int32_t
sem_node_realloc_leaves(struct sem_node *node)
{
    int32_t new_size = node->leaves_max_num * 2;

    node->leaves = realloc(node->leaves, new_size * sizeof(*node->leaves));
    if (!node->leaves) {
        return -1;
    }

    node->leaves_max_num = new_size;

    return 0;
}

static struct sem_node *
sem_node_tree_add_leaf(struct sem_node *parent, enum sem_node_type leaf_type)
{
    if (!parent) {
        return NULL;
    }

    if (!parent->leaves) {
        int8_t err = sem_node_alloc_leaves(parent);
        if (err) {
            return NULL;
        }
    }

    parent->leaves_num++;
    if (parent->leaves_num >= parent->leaves_max_num) {
        int8_t err = sem_node_realloc_leaves(parent);
        if (err) {
            return NULL;
        }
    }

    struct sem_node *new_leaf = &parent->leaves[parent->leaves_num - 1];
    sem_node_init(new_leaf);
    new_leaf->type = leaf_type;
    new_leaf->root = parent;

    return new_leaf;
}

int8_t
sem_analyzer_init(
        struct sem_analyzer *res,
        struct lex_parser   *lexer,
        struct syn_parser   *syntaxer)
{
    if (!res || !lexer || !syntaxer) {
        return -1;
    }

    res->lexer = lexer;
    res->syntaxer = syntaxer;
    sem_node_init(&res->tree);
    res->tree.type = SEM_ROOT;
    res->cur_node = &res->tree;

    return 0;
}

void
sem_analyzer_free(struct sem_analyzer *analyzer)
{
    if (!analyzer) {
        return;
    }

    sem_node_tree_free(&analyzer->tree);
}

static int8_t
sem_symbol_to_action(enum lex_terminal symbol, enum sem_node_type *res)
{
    switch (symbol) {
        case LEX_INC:
            *res = SEM_ACTION_INC;
            break;

        case LEX_DEC:
            *res = SEM_ACTION_DEC;
            break;

        case LEX_LEFT:
            *res = SEM_ACTION_LEFT;
            break;

        case LEX_RIGHT:
            *res = SEM_ACTION_RIGHT;
            break;

        case LEX_INPUT:
            *res = SEM_ACTION_INPUT;
            break;

        case LEX_OUTPUT:
            *res = SEM_ACTION_OUTPUT;
            break;

        case LEX_UP:
            *res = SEM_ACTION_UP;
            break;

        case LEX_DOWN:
            *res = SEM_ACTION_DOWN;
            break;

        case LEX_FUNC_CALL:
            *res = SEM_ACTION_FUNC_CALL;
            break;

        case LEX_RETURN:
            *res = SEM_ACTION_RETURN;
            break;

        case LEX_SYS_CALL:
            *res = SEM_ACTION_SYS_CALL;
            break;

        default:
            return -1;
            break;
    }

    return 0;
}

static void
sem_analyzer_go_to_parent_node(struct sem_analyzer *analyzer)
{
    struct sem_node *parent = analyzer->cur_node->root;
    analyzer->cur_node = parent;
}

static int8_t
sem_analyzer_add_node(struct sem_analyzer *analyzer, enum sem_node_type node_type)
{
    struct sem_node *new_node = sem_node_tree_add_leaf(analyzer->cur_node, node_type);
    if (!new_node) {
        return -1;
    }

    analyzer->cur_node = new_node;

    return 0;
}

static int8_t
sem_analyzer_process_func(struct sem_analyzer *analyzer)
{
    return sem_analyzer_add_node(analyzer, SEM_FUNC);
}

static int8_t
sem_analyzer_process_cyc(struct sem_analyzer *analyzer)
{
    return sem_analyzer_add_node(analyzer, SEM_CYC);
}

static int8_t
sem_analyzer_process_cyc_start(struct sem_analyzer *analyzer)
{
    int8_t err = sem_analyzer_add_node(analyzer, SEM_CYC_START);
    if (err) {
        return -1;
    }

    sem_analyzer_go_to_parent_node(analyzer);

    err = sem_analyzer_add_node(analyzer, SEM_CYC_BODY);
    if (err) {
        return -1;
    }

    return 0;
}

static int8_t
sem_analyzer_process_cyc_end(struct sem_analyzer *analyzer)
{
    sem_analyzer_go_to_parent_node(analyzer);

    int8_t err = sem_analyzer_add_node(analyzer, SEM_CYC_END);
    if (err) {
        return -1;
    }

    sem_analyzer_go_to_parent_node(analyzer);
    return 0;
}

static int8_t
sem_analyzer_process_symbol(struct sem_analyzer *analyzer, enum lex_terminal symbol)
{
    enum sem_node_type action = 0;
    int8_t err = sem_symbol_to_action(symbol, &action);
    if (err){
        return -1;
    }

    err = sem_analyzer_add_node(analyzer, action);
    if (err) {
        return -1;
    }

    sem_analyzer_go_to_parent_node(analyzer);
    return 0;
}

static int8_t
sem_analyzer_process_node(
        struct sem_analyzer *analyzer,
        enum syn_state       state,
        enum lex_terminal    lex)
{
    if (!analyzer) {
        return -1;
    }

    switch (state) {
        case SYN_START: case SYN_FUNC_BODY: case SYN_CONSTRUCT:
        case SYN_COMMENT_START: case SYN_COMMENT:
            break;

        case SYN_FUNC:
            return sem_analyzer_process_func(analyzer);
            break;

        case SYN_SYMBOL:
            return sem_analyzer_process_symbol(analyzer, lex);
            break;

        case SYN_CYC:
            return sem_analyzer_process_cyc(analyzer);
            break;

        case SYN_CYC_START:
            return sem_analyzer_process_cyc_start(analyzer);
            break;

        case SYN_CYC_END:
        {
            int8_t err = sem_analyzer_process_cyc_end(analyzer);
            if (err) {
                return -1;
            }
            return sem_analyzer_process_cyc_end(analyzer);
            break;
        }

        case SYN_DELIM:
            sem_analyzer_go_to_parent_node(analyzer);
            break;

        default:
            return -1;
            break;
    }

    return 0;
}

int8_t
sem_analyzer_process(struct sem_analyzer *analyzer)
{
    if (!analyzer || !analyzer->lexer || !analyzer->syntaxer) {
        return -1;
    }

    struct lex_parser *lexer = analyzer->lexer;
    struct syn_parser *syntaxer = analyzer->syntaxer;

    enum syn_state cur_state = syn_parser_cur_state(syntaxer);
    if (cur_state != SYN_START) {
        return -1;
    }

    while (cur_state != SYN_END) {
        if (syntaxer->need_next_lex) {
            int8_t err = lex_parser_read(lexer);
            if (err) {
                return -1;
            }
        }
        enum lex_terminal cur_lex = lexer->next;

        int8_t err = sem_analyzer_process_node(analyzer, cur_state, cur_lex);
        if (err) {
            return -1;
        }

        err = syn_parser_process_lex(syntaxer, cur_lex);
        if (err) {
            return -1;
        }
        cur_state = syn_parser_cur_state(syntaxer);
    }

    return 0;
}

struct sem_node_depth {
    struct sem_node *node;
    int32_t depth;
};

int8_t
sem_analyzer_tree_print(struct sem_analyzer *analyzer)
{
    if (!analyzer) {
        return -1;
    }

    int32_t stack_max_size = 32;

    struct sem_node_depth *stack = calloc(stack_max_size, sizeof(*stack));
    if (!stack) {
        return -1;
    }

    stack[0].node = &analyzer->tree;
    int32_t stack_len = 1;

    while (stack_len > 0) {
        struct sem_node_depth cur_elem = stack[stack_len - 1];
        stack_len--;

        for (int32_t i = 0; i < cur_elem.depth - 1; i++) {
            printf("    ");
        }
        if (cur_elem.depth > 0) {
            printf("└-- ");
        }
        printf("%d\n", (int) cur_elem.node->type);

        for (int32_t i = 0; i < cur_elem.node->leaves_num; i++) {
            struct sem_node_depth new_elem = {0};
            new_elem.node = &cur_elem.node->leaves[i];
            new_elem.depth = cur_elem.depth + 1;

            stack_len++;
            if (stack_len >= stack_max_size) {
                stack_max_size *= 2;
                stack = realloc(stack, stack_max_size * sizeof(*stack));
                if (!stack) {
                    return -1;
                }
            }

            stack[stack_len - 1] = new_elem;
        }
    }

    free(stack);

    return 0;
}

int8_t
sem_analyzer_err_fprint(struct sem_analyzer *analyzer, FILE *file)
{
    if (!analyzer || !analyzer->lexer || !analyzer->syntaxer) {
        fprintf(file, "Internal complier error.\n");
        return -1;
    }

    struct lex_parser *lexer = analyzer->lexer;
    struct syn_parser *syntaxer = analyzer->syntaxer;

    enum syn_state cur_state = syn_parser_cur_state(syntaxer);
    enum lex_terminal lex = lexer->next;
    char ch = lexer->parsed;

    int32_t res = fprintf(
            file,
            "Syntax analazyer state code %d, lexeme code %d.\n",
            cur_state, lex);
    if (res < 0) {
        return -1;
    }

    const char *err_msg = "Error: unknown semantic error near '%c' symbol.\n";

    if (lex == LEX_ERR) {
        err_msg = "Error: lexical error while reading source code near '%c' symbol.\n";
    } else if (lex == LEX_UNK) {
        err_msg = "Error: unknown lexeme near '%c' symbol.\n";
    } else if (lex == LEX_EOF) {
        err_msg = "Error: unexpected EOF near '%c' symbol.\n";
    } else if (cur_state == SYN_ERR) {
        err_msg = "Error: syntax error near '%c' symbol.\n";
    }

    res = fprintf(file, err_msg, ch);
    if (res < 0) {
        return -1;
    }

    return 0;
}

int8_t
sem_node_is_action(enum sem_node_type node_type)
{
    return node_type == SEM_ACTION_INC
            || node_type == SEM_ACTION_DEC
            || node_type == SEM_ACTION_LEFT
            || node_type == SEM_ACTION_RIGHT
            || node_type == SEM_ACTION_INPUT
            || node_type == SEM_ACTION_OUTPUT
            || node_type == SEM_ACTION_UP
            || node_type == SEM_ACTION_DOWN
            || node_type == SEM_ACTION_FUNC_CALL
            || node_type == SEM_ACTION_RETURN
            || node_type == SEM_ACTION_SYS_CALL;
}