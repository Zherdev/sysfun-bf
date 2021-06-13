/*
 * sysfun-bf interpreter.
 *
 * Zherdev, 2021
 */

#include "interpreter.h"

#include <stdint.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    if (argc != 2 || strlen(argv[1]) == 0) {
        return -1;
    }
    const char *filename = argv[1];

    struct interpreter interp = {0};
    int8_t err = interpreter_init(&interp, filename);
    if (err) {
        return -1;
    }

    err = interpreter_read(&interp);
    if (err) {
        return -2;
    }

    err = interpreter_run(&interp);
    if (err) {
        return -3;
    }

    err = interpreter_free(&interp);
    if (err) {
        return -4;
    }

    return 0;
}