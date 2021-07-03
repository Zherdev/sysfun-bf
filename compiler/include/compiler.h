/*
 * Zherdev, 2021
 */

#ifndef COMPILER_H
#define COMPILER_H

#include "parser.h"

#include <stdint.h>

struct complier {
    struct parser    parser;
    struct sem_node *sem_root;
};

int8_t
compiler_init(struct compiler *compiler, const char *filename);

int8_t
compiler_free(struct compiler *compiler);

int8_t
compiler_read(struct interpreter *compiler);

int8_t
compiler_compile(struct interpreter *compiler);

#endif // COMPILER_H