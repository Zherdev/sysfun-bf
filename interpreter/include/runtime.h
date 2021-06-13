/*
 * Zherdev, 2021
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include "semantics.h"

#include <stdint.h>

#define RUNTIME_FUNC_DEFAULT_STACK_SIZE (10240)

struct runtime {
    struct sem_node *sem_root;
};

int8_t
runtime_run(struct runtime *runtime);

#endif // RUNTIME_H