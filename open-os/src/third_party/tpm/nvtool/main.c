// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bitstring.h"
#include "commands.h"
#include "tpm_nv.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>

#define PROGNAME "tpm-nvtool"

static struct option
long_options[] = {
    { "define",         no_argument,       0, 'D' },
    { "file",           required_argument, 0, 'f' },
    { "help",           no_argument,       0, 'h' },
    { "hexdump",        no_argument,       0, 'x' },
    { "index",          required_argument, 0, 'I' },
    { "index_password", required_argument, 0, 'i' },
    { "list",           no_argument,       0, 'l' },
    { "offset",         required_argument, 0, 'O' },
    { "owner_password", required_argument, 0, 'o' },
    { "password",       required_argument, 0, '0' },
    { "pcr",            required_argument, 0, 'p' },
    { "permissions",    required_argument, 0, 'P' },
    { "read",           no_argument,       0, 'r' },
    { "release",        no_argument,       0, 'R' },
    { "size",           required_argument, 0, 's' },
    { "string",         required_argument, 0, 'd' },
    { "write",          no_argument,       0, 'w' },
    { "writezero",      no_argument,       0, 'z' },
    { NULL, 0, 0, 0 },
};

static const char* options = "0:DI:O:P:Rd:f:hi:lo:p:rs:wxz";

static void
help_brief(void)
{
    TNV_stderr("Try %s --help for more information.\n", PROGNAME);
}

static void
help_long(void)
{
    const char* help_message =
#include "help/help.h"
    ;
    TNV_stderr("%s", help_message);
}

int
main(int argc, char** argv)
{
    tnv_args_t tnv_args;
    tnv_context_t* t = NULL;
    char* tss_server = NULL;
    uint64_t cmdmap = 0ULL;
    int c, option_index = 0, ret = 0;
    int data_fd = -1;

    if (argc <= 1) {
        help_brief();
        exit(1);
    }

    memset(&tnv_args, 0, sizeof(tnv_args));
    tnv_args.data_fd = -1;

    while ((c = getopt_long(argc, argv, options, long_options, &option_index))
           != -1) {
        switch (c) {
            case '0':
                tnv_args.password = optarg;
                tnv_args.flags |= TNV_FLAG_RWAUTH;
                break;
            case 'D':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_DEFINE);
                break;
            case 'I': {
                errno = 0;
                char* end;
                uint64_t tmp_index = strtoull(optarg, &end, 0);
                if (errno || (end != optarg + strlen(optarg)) ||
                    (tmp_index > UINT_MAX) || (tmp_index == 0)) {
                    TNV_stderr("Invalid index %s.\n", optarg);
                    exit(1);
                }
                tnv_args.index = (uint32_t)tmp_index;
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_INDEX);
                break;
            }
            case 'O':
                errno = 0;
                char* end;
                uint64_t tmp_offset = strtoull(optarg, &end, 0);
                if (errno || (end != optarg + strlen(optarg)) ||
                    (tmp_offset > UINT_MAX)) {
                    TNV_stderr("Invalid offset %s.\n", optarg);
                    exit(1);
                }
                tnv_args.offset = (uint32_t)tmp_offset;
                break;
            case 'P': {
                int i;
                char* aperm = strtok((char*)optarg, ",");
                while (aperm) {
                   for (i = 0; TPM_NV_PER_table[i].permission_name; i++) {
                       if (strcasecmp(TPM_NV_PER_table[i].permission_name,
                                      aperm) == 0) {
                           if (TPM_NV_PER_table[i].allowed == TRUE) {
                               tnv_args.permissions |=
                                   TPM_NV_PER_table[i].permission_value;
                           }
                       }
                   }
                   aperm = strtok(NULL, ","); 
                }
                break;
            }
            case 'R':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_RELEASE);
                break;
            case 'd':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_DATA);
                if (*optarg == '\0') {
                    TNV_stderr("Attempt to write an empty string.\n");
                    exit(1);
                }
                tnv_args.data = optarg;
                break;
            case 'f':
                tnv_args.data = optarg;
                tnv_args.flags |= TNV_FLAG_FILEDATA;
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_FILE);
                break;
            case 'h':
                help_long();
                exit(0);
            case 'i':
                tnv_args.index_password = optarg;
                break;
            case 'l':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_LIST);
                break;
            case 'o':
                tnv_args.owner_password = optarg;
                break;
            case 'p': {
                errno = 0;
                char* end;
                uint64_t tmp_pcr = strtoull(optarg, &end, 0);
                if (errno || (end != optarg + strlen(optarg)) ||
                    (tmp_pcr >= TNV_MAX_PCRS)) {
                    TNV_stderr("Invalid PCR index %s.\n", optarg);
                    exit(1);
                }
                bit_set(tnv_args.pcrs_selected.bitmap, tmp_pcr);
                if ((uint32_t)tmp_pcr > tnv_args.pcrs_selected.highest) {
                    tnv_args.pcrs_selected.highest = (uint32_t)tmp_pcr;
                }
                tnv_args.pcrs_selected.count++;
                break;
            }
            case 'r':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_READ);
                break;
            case 's': {
                errno = 0;
                char* end;
                uint64_t tmp_size = strtoull(optarg, &end, 0);
                if (errno || (end != optarg + strlen(optarg)) ||
                    (tmp_size > UINT_MAX)) {
                    TNV_stderr("Invalid size %s.\n", optarg);
                    exit(1);
                }
                tnv_args.size = (uint32_t)tmp_size;
                break;
            }
            case 'w':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_WRITE);
                break;
            case 'x':
                tnv_args.flags |= TNV_FLAG_HEXDUMP;
                break;
            case 'z':
                cmdmap |= CMD_BITMAP(CMDBIT_TNV_WRITEZERO);
                break;
            default:
                help_brief();
                exit(1);
        }
    }

    if (optind != argc) {
        help_brief();
        exit(1);
    }

    switch (cmdmap) {
        case CMD_TNV_DEFINE:
            tnv_args.flags |= (TNV_FLAG_CREATE | TNV_FLAG_NEEDOWNER);
            break;
        case CMD_TNV_RELEASE:
            tnv_args.flags |= (TNV_FLAG_DESTROY | TNV_FLAG_NEEDOWNER);
            break;
        case CMD_TNV_LIST:
            tnv_args.flags |= TNV_FLAG_NONSPECIFIC;
            break;
        case CMD_TNV_LIST_INDEX:
        case CMD_TNV_READ:
            break;
        case CMD_TNV_WRITE: {
            size_t dataLength = strlen(tnv_args.data);
            if ((tnv_args.size == 0) || (tnv_args.size > dataLength)) {
                tnv_args.size = dataLength;
            }
            break;
        }
        case CMD_TNV_WRITE_FILE: {
            struct stat stbuf;
            char fileBuffer[TNV_MAX_NV_SIZE];
            int fd = open(tnv_args.data, O_RDONLY);
            ret = -1;
            if (fd < 0) {
                TNV_stderr("Failed to open %s for reading (%s).\n",
                           tnv_args.data, strerror(errno));
                goto out;
            }
            if (fstat(fd, &stbuf) < 0) {
                TNV_stderr("Failed to stat %s (%s).\n", tnv_args.data,
                           strerror(errno));
                goto out;
            }
            if (stbuf.st_size > TNV_MAX_NV_SIZE) {
                TNV_stderr("Will not read files larger than %u bytes.\n",
                           TNV_MAX_NV_SIZE);
                goto out;
            }
            if (stbuf.st_size == 0) {
                TNV_stderr("Zero length file %s.\n", tnv_args.data);
                goto out;
            }
            if (!S_ISREG(stbuf.st_mode)) {
                TNV_stderr("Will not read from a non-regular file.\n");
                goto out;
            }
            if (read(fd, fileBuffer, stbuf.st_size) != stbuf.st_size) {
                TNV_stderr("Failed to read file content (%s).\n",
                           strerror(errno));
                goto out;
            }
            tnv_args.data = &fileBuffer[0];
            if ((tnv_args.size == 0) || (tnv_args.size > stbuf.st_size)) {
                tnv_args.size = stbuf.st_size;
            }
            break;
        }
        case CMD_TNV_WRITEZERO:
            tnv_args.offset = 0;
            tnv_args.size = 0;
            break;
        default:
            help_brief();
            exit(1);
    }

    tnv_args.tss_version = TSS_TSPATTRIB_CONTEXT_VERSION_V1_1;

    t = tnv_open_context(tss_server, &tnv_args);
    if (t == NULL) {
        exit(1);
    }

    switch (cmdmap) {
        case CMD_TNV_DEFINE:
            ret = tnv_define(t, &tnv_args);
            break;
        case CMD_TNV_LIST:
            ret = tnv_list(t, &tnv_args);
            break;
        case CMD_TNV_LIST_INDEX:
            ret = tnv_list(t, &tnv_args);
            break;
        case CMD_TNV_READ:
            ret = tnv_read(t, &tnv_args);
            break;
        case CMD_TNV_RELEASE:
            ret = tnv_release(t, &tnv_args);
            break;
        case CMD_TNV_WRITE:
            ret = tnv_write(t, &tnv_args);
            break;
        case CMD_TNV_WRITEZERO:
            ret = tnv_write(t, &tnv_args);
            break;
  }

out:

    if (data_fd >= 0) {
        close(data_fd);
    }

    tnv_close_context(&t);

    exit(ret);
}
