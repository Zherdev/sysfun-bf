/*
 * Zherdev, 2021
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "runtime.h"

#include <stdint.h>

struct interpreter {
    struct parser  parser;
    struct runtime runtime;
};

int8_t
interpreter_init(struct interpreter *interp, const char *filename);

int8_t
interpreter_free(struct interpreter *interp);

int8_t
interpreter_read(struct interpreter *interp);

int8_t
interpreter_run(struct interpreter *interp);

#endif // INTERPRETER_H