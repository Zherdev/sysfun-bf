/*
 * See parser/include/lex.h for details.
 *
 * Zherdev, 2021
 */

#include "lex.h"

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
lex_parser_init(struct lex_parser *parser, int32_t fd)
{
    if (!parser) {
        return -1;
    }

    parser->fd = fd;
    parser->next = LEX_UNK;
    parser->parsed = 0;

    return 0;
}

static int8_t
lex_fd_is_ok(int32_t fd)
{
    return fcntl(fd, F_GETFD) != -1;
}

static int8_t
lex_parser_parse_char(struct lex_parser *parser, char ch)
{
    switch (ch) {
        case '+':
            parser->next = LEX_INC;
            break;

        case '-':
            parser->next = LEX_DEC;
            break;

        case '<':
            parser->next = LEX_LEFT;
            break;

        case '>':
            parser->next = LEX_RIGHT;
            break;

        case '[':
            parser->next = LEX_CYC_START;
            break;

        case ']':
            parser->next = LEX_CYC_END;
            break;

        case ',':
            parser->next = LEX_INPUT;
            break;

        case '.':
            parser->next = LEX_OUTPUT;
            break;

        case '^':
            parser->next = LEX_UP;
            break;

        case 'v':
            parser->next = LEX_DOWN;
            break;

        case ':':
            parser->next = LEX_FUNC_CALL;
            break;

        case ';':
            parser->next = LEX_RETURN;
            break;

        case '%':
            parser->next = LEX_SYS_CALL;
            break;

        case ' ':
            parser->next = LEX_COMMENT;
            break;

        case '\n':
            parser->next = LEX_DELIM;
            break;

        default:
            parser->next = LEX_UNK;
            break;
    }

    return 0;
}

int8_t
lex_parser_read(struct lex_parser *parser)
{
    if (!parser) {
        return -1;
    }

    if (!lex_fd_is_ok(parser->fd)) {
        parser->next = LEX_ERR;
        return -1;
    }

    int32_t res = read(parser->fd, &parser->parsed, sizeof(char));
    switch (res) {
        case sizeof(char):
            return lex_parser_parse_char(parser, parser->parsed);
            break;

        case 0:
            parser->next = LEX_EOF;
            return 0;
            break;

        default:
            parser->next = LEX_ERR;
            return -1;
            break;
    }

    return -1;
}

int8_t
lex_is_common_symbol(enum lex_terminal lex)
{
    return lex == LEX_INC
            || lex == LEX_DEC
            || lex == LEX_LEFT
            || lex == LEX_RIGHT
            || lex == LEX_INPUT
            || lex == LEX_OUTPUT
            || lex == LEX_UP
            || lex == LEX_DOWN
            || lex == LEX_FUNC_CALL
            || lex == LEX_RETURN
            || lex == LEX_SYS_CALL;
}