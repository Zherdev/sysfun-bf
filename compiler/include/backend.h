/*
 * Zherdev, 2021
 */

#ifndef BACKEND_H
#define BACKEND_H

#include <stdint.h>

typedef int8_t (*backend_open_file)(void *back_, const char *filename);
typedef int8_t (*backend_close_file)(void *back_);
typedef int8_t (*backend_prepare)(void *back_);
typedef int8_t (*backend_write_)(void *back_);

#endif // BACKEND_H