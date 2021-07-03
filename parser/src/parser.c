/*
 * See parser/include/parser.h for details.
 *
 * Zherdev, 2021
 */

#include "parser.h"

#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __linux

#include <sys/io.h>

#elif defined(__osx)

#include <sys/uio.h>
#endif

int8_t
parser_init(struct parser *parser, const char *filename)
{
    if (!parser || !filename) {
        return -1;
    }

    struct lex_parser   *lexer    = &parser->lexer;
    struct syn_parser   *syntaxer = &parser->syntaxer;
    struct sem_analyzer *analyzer = &parser->analyzer;

    int8_t fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return -1;
    }
    parser->fd = fd;

    int8_t err = lex_parser_init(lexer, fd);
    if (err) {
        return -1;
    }


    err = syn_parser_init(syntaxer);
    if (err) {
        return -1;
    }


    err = sem_analyzer_init(analyzer, lexer, syntaxer);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
parser_free(struct parser *parser)
{
    if (!parser) {
        return 0;
    }

    struct syn_parser   *syntaxer = &parser->syntaxer;
    struct sem_analyzer *analyzer = &parser->analyzer;


    sem_analyzer_free(analyzer);
    syn_parser_free(syntaxer);

    int8_t err = close(parser->fd);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
parser_process_file(struct parser *parser)
{
    if (!parser) {
        return -1;
    }

    struct sem_analyzer *analyzer = &parser->analyzer;

    int8_t err = sem_analyzer_process(analyzer);
    if (err) {
        return -1;
    }

    return 0;
}

int8_t
parser_err_to_stderr(struct parser *parser)
{
    if (!parser) {
        return -1;
    }

    struct sem_analyzer *analyzer = &parser->analyzer;
    return sem_analyzer_err_fprint(analyzer, stderr);
}