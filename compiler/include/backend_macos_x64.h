/*
 * Zherdev, 2021
 */

#ifndef BACKEND_MACOS_X64_H
#define BACKEND_MACOS_X64_H

#include <stdint.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <mach-o/reloc.h>

struct backend_macos_x64 {
    int32_t fd;

    struct mach_header        header;
    struct segment_command_64 cmd_segment;
    struct section_64         sec_text;
    struct section_64         sec_data;
    struct symtab_command     cmd_symtab;

    uint8_t *bytes_text;
    uint8_t *bytes_data;
    uint8_t *bytes_symtab;
};

int8_t
backend_macos_x64_open_file(void *back_, const char *filename);

int8_t
backend_macos_x64_close_file(void *back_);

int8_t
backend_macos_x64_prepare(void *back_);

int8_t
backend_macos_x64_write_(void *back_);

#endif // BACKEND_H