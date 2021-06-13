/*
 * See interpreter/include/runtime.h for details.
 *
 * Zherdev, 2021
 */

#include "runtime.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

struct runtime_func {
    struct sem_node *sem_node;
    uint32_t head_pos;
    uint32_t func_pos;
    uint8_t  done;
    uint8_t  return_code;
    uint8_t  buff[RUNTIME_FUNC_DEFAULT_STACK_SIZE];
};

static int8_t
runtime_func_run(struct runtime_func *func);

static int8_t
runtime_func_run_node(struct runtime_func *func, struct sem_node *node);

static struct sem_node *
runtime_func_get_subfunc_node(struct runtime_func *func)
{
    struct sem_node *root = func->sem_node->root;

    if (root->leaves_num <= func->func_pos) {
        return NULL;
    }

    return &root->leaves[func->func_pos];
}

static int8_t
runtime_func_run_action(struct runtime_func *func, enum sem_node_type action)
{
    switch (action) {
        case SEM_ACTION_INC:
            func->buff[func->head_pos]++;
            return 0;
            break;

        case SEM_ACTION_DEC:
            func->buff[func->head_pos]--;
            return 0;
            break;

        case SEM_ACTION_LEFT:
            func->head_pos--;
            return 0;
            break;

        case SEM_ACTION_RIGHT:
            func->head_pos++;
            return 0;
            break;

        case SEM_ACTION_INPUT:
            func->buff[func->head_pos] = getc(stdin);
            return 0;
            break;

        case SEM_ACTION_OUTPUT:
            if (putc(func->buff[func->head_pos], stdout) == EOF) {
                return -1;
            }
            return 0;
            break;

        case SEM_ACTION_UP:
            func->func_pos--;
            return 0;
            break;

        case SEM_ACTION_DOWN:
            func->func_pos++;
            return 0;
            break;

        case SEM_ACTION_RETURN:
            func->done = 1;
            func->return_code = func->buff[func->head_pos];
            return 0;
            break;

        case SEM_ACTION_FUNC_CALL:
        {
            struct runtime_func subfunc = {0};

            subfunc.sem_node = runtime_func_get_subfunc_node(func);
            if (!subfunc.sem_node) {
                return -1;
            }

            int8_t err = runtime_func_run(&subfunc);
            if (err) {
                return -1;
            }

            func->buff[func->head_pos] = subfunc.return_code;
            return 0;
            break;
        }

        case SEM_ACTION_SYS_CALL:
        {
            return -1;
        }

        default:
            return -1;
            break;
    }

    return -1;
}

static int8_t
runtime_func_run_cyc(struct runtime_func *func, struct sem_node *cyc)
{
    struct sem_node *body = &cyc->leaves[1];

    while (func->buff[func->head_pos]) {
        for (int32_t i = 0; i < body->leaves_num; i++) {
            struct sem_node *leaf = &body->leaves[i];

            int8_t err = runtime_func_run_node(func, leaf);
            if (err) {
                return -1;
            }
            if (func->done) {
                return 0;
            }
        }
    }

    return 0;
}

static int8_t
runtime_func_run_node(struct runtime_func *func, struct sem_node *node)
{
    if (node->type == SEM_CYC) {
        int8_t err = runtime_func_run_cyc(func, node);
        if (err) {
            return -1;
        }
        if (func->done) {
            return 0;
        }
    }

    if (sem_node_is_action(node->type)) {
        int8_t err = runtime_func_run_action(func, node->type);
        if (err) {
            return -1;
        }
    }

    return 0;
}

static int8_t
runtime_func_run(struct runtime_func *func)
{
    if (!func) {
        return -1;
    }

    struct sem_node *sem_node = func->sem_node;
    struct sem_node *leaves = sem_node->leaves;

    for (int32_t i = 0; i < sem_node->leaves_num; i++) {
        struct sem_node *leaf = &leaves[i];

        int8_t err = runtime_func_run_node(func, leaf);
        if (err) {
            return -1;
        }
        if (func->done) {
            return 0;
        }
    }

    func->done = 1;
    func->return_code = func->buff[func->head_pos];
    return 0;
}

int8_t
runtime_run(struct runtime *runtime)
{
    if (!runtime || !runtime->sem_root) {
        return -1;
    }

    if (runtime->sem_root->leaves_num == 0) {
        return 0;
    }

    struct runtime_func main = {0};
    main.sem_node = &runtime->sem_root->leaves[0];

    return runtime_func_run(&main);
}