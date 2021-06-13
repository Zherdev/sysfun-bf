/*
 * See interpreter/include/interpreter.h for details.
 *
 * Zherdev, 2021
 */

#include "interpreter.h"

int8_t
interpreter_init(struct interpreter *interp, const char *filename)
{
    if (!interp || !filename) {
        return -1;
    }

    int8_t err = parser_init(&interp->parser, filename);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
interpreter_free(struct interpreter *interp)
{
    if (!interp) {
        return 0;
    }

    int8_t err = parser_free(&interp->parser);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
interpreter_read(struct interpreter *interp)
{
    if (!interp) {
        return 0;
    }

    struct parser  *parser  = &interp->parser;
    struct runtime *runtime = &interp->runtime;

    int8_t err = parser_process_file(parser);
    if (err) {
        parser_err_to_stderr(parser);
        return -1;
    }

    runtime->sem_root = &parser->analyzer.tree;

    return 0;
}

int8_t
interpreter_run(struct interpreter *interp)
{
    if (!interp) {
        return 0;
    }
    struct runtime *runtime = &interp->runtime;

    return runtime_run(runtime);
}