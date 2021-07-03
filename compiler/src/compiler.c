/*
 * See interpreter/include/interpreter.h for details.
 *
 * Zherdev, 2021
 */

#include "compiler.h"

int8_t
compiler_init(struct compiler *compiler, const char *filename)
{
    if (!compiler || !filename) {
        return -1;
    }

    int8_t err = parser_init(&compiler->parser, filename);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
compiler_free(struct compiler *compiler)
{
    if (!compiler) {
        return 0;
    }

    int8_t err = parser_free(&compiler->parser);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
compiler_read(struct compiler *compiler)
{
    if (!compiler) {
        return 0;
    }

    struct parser  *parser  = &compiler->parser;

    int8_t err = parser_process_file(parser);
    if (err) {
        parser_err_to_stderr(parser);
        return -1;
    }

    compiler->sem_root = &parser->analyzer.tree;

    return 0;
}

int8_t
compiler_compile(struct compiler *compiler)
{
    if (!compiler) {
        return 0;
    }
    struct runtime *runtime = &interp->runtime;

    return runtime_run(runtime);
}