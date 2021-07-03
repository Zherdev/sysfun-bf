/*
 * See compiler/include/backend_macos_x64.h for details.
 *
 * Zherdev, 2021
 */

#include "backend_macos_x64.h"

#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>

int8_t
backend_macos_x64_open_file(void *back_, const char *filename)
{
    struct backend_macos_x64 *back = back_;
    if (!back || !filename) {
        return -1;
    }

    int32_t fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT);
    if (fd == -1 || fcntl(fd, F_GETFD) == -1) {
        return -1;
    }
    back->fd = fd;

    return 0;
}

int8_t
backend_macos_x64_close_file(void *back_)
{
    struct backend_macos_x64 *back = back_;
    if (!back) {
        return -1;
    }

    int8_t err = close(back->fd);
    if (err) {
        return -1;
    }

    return 0;
}

static uint64_t
backend_macos_x64_sizeofcmds(struct backend_macos_x64 *back)
{
    return sizeof(back->cmd_segment)
            + sizeof(back->sec_text)
            + sizeof(back->sec_data)
            + sizeof(back->cmd_symtab);
}

static void
backend_macos_x64_prepare_header(struct backend_macos_x64 *back)
{
    struct mach_header *header = &back->header;

    memset(header, 0, sizeof(*header));

    header->magic = MH_MAGIC_64;
    header->cputype = CPU_TYPE_X86_64;
    header->cpusubtype = CPU_SUBTYPE_X86_64_ALL;
    header->filetype = MH_OBJECT;
    header->ncmds = 2;
    header->sizeofcmds = backend_macos_x64_sizeofcmds(back);
    header->flags = 0;
}

static uint64_t
backend_macos_x64_seg_cmdsize(struct backend_macos_x64 *back)
{
    return sizeof(back->cmd_segment)
            + sizeof(back->sec_data)
            + sizeof(back->sec_text);
}

static uint64_t
backend_macos_x64_alignment(uint64_t offset)
{
    return (8 - (offset % 8)) % 8;
}

static uint64_t
backend_macos_x64_alignment_after_header(struct backend_macos_x64 *back)
{
    return backend_macos_x64_alignment(sizeof(back->header));
}

static uint64_t
backend_macos_x64_seg_fileoff(struct backend_macos_x64 *back)
{
    return sizeof(back->header)
            + backend_macos_x64_alignment_after_header(back)
            + back->header.sizeofcmds;
}

static void
backend_macos_x64_prepare_cmd_seg(struct backend_macos_x64 *back)
{
    struct segment_command_64 *cmd = &back->cmd_segment;

    memset(cmd, 0, sizeof(*cmd));

    cmd->cmd = LC_SEGMENT_64;
    cmd->cmdsize = backend_macos_x64_seg_cmdsize(back);
    cmd->vmaddr = NULL;
    cmd->vmsize = cmd->cmdsize;
    cmd->fileoff = backend_macos_x64_seg_fileoff(back);
    cmd->filesize = 0; // will be calculated later at compilation.
    cmd->maxprot = VM_PROT_ALL;
    cmd->initprot = VM_PROT_ALL;
    cmd->nsects = 2;
    cmd->flags = 0;
}

static void
backend_macos_x64_prepare_sec_text(struct backend_macos_x64 *back)
{
    struct section_64 *sec = &back->sec_text;

    memset(sec, 0, sizeof(*sec));

    memcpy(sec->sectname, SECT_TEXT, sizeof(sec->sectname));
    memcpy(sec->segname, SEG_TEXT, sizeof(sec->sectname));
    sec->addr = NULL;
    sec->size = 0; // will be calculated later at compilation.
    sec->offset = back->cmd_segment.fileoff;
    sec->align = 1;
    sec->reloff = 0; // will be calculated later at compilation.
    sec->nreloc = 2;
    sec->flags = S_REGULAR
            | S_ATTR_PURE_INSTRUCTIONS
            | S_ATTR_SOME_INSTRUCTIONS
            | S_ATTR_EXT_RELOC
            | S_ATTR_LOC_RELOC;
}

static void
backend_macos_x64_prepare_sec_data(struct backend_macos_x64 *back)
{
    struct section_64 *sec = &back->sec_data;

    memset(sec, 0, sizeof(*sec));

    memcpy(sec->sectname, SECT_DATA, sizeof(sec->sectname));
    memcpy(sec->segname, SEG_DATA, sizeof(sec->sectname));
    sec->addr = NULL; // will be calculated later at compilation.
    sec->size = 0; // will be calculated later at compilation.
    sec->offset = 0; // will be calculated later at compilation.
    sec->align = 1;
    sec->reloff = 0;
    sec->nreloc = 0;
    sec->flags = S_REGULAR;
}

static void
backend_macos_x64_prepare_cmd_symtab(struct backend_macos_x64 *back)
{
    struct symtab_command *cmd = &back->cmd_symtab;

    memset(cmd, 0, sizeof(*cmd));

    cmd->cmd = LC_SYMTAB;
    cmd->cmdsize = sizeof(*cmd);
    cmd->symoff = 0; // will be calculated later at compilation.
    cmd->nsyms = 3;
    cmd->stroff = 0; // will be calculated later at compilation.
    cmd->strsize = 0;
}

int8_t
backend_macos_x64_prepare(void *back_)
{
    struct backend_macos_x64 *back = back_;
    if (!back) {
        return -1;
    }

    backend_macos_x64_prepare_header(back);
    backend_macos_x64_prepare_cmd_seg(back);
    backend_macos_x64_prepare_sec_text(back);
    backend_macos_x64_prepare_sec_data(back);
    backend_macos_x64_prepare_cmd_symtab(back);

    return 0;
}

typedef int8_t (*backend_write_)(void *back_);
