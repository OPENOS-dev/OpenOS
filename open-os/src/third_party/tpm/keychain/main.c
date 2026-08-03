// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bitstring.h"
#include "commands.h"
#include "tpm_keychain.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <getopt.h>
#include <errno.h>

#define PROGNAME "tpm-keychain"

static struct option
long_options[] = {
    { "add",               required_argument, 0, 'A' },
    { "blob",              no_argument,       0, 'b' },
    { "create",            no_argument,       0, 'C' },
    { "destroy",           no_argument,       0, 'D' },
    { "force",             no_argument,       0, 'f' },
    { "help",              no_argument,       0, 'h' },
    { "key_password",      required_argument, 0, 'k' },
    { "keychain_password", required_argument, 0, 'K' },
    { "list",              no_argument,       0, 'l' },
    { "new_password",      required_argument, 0, 'n' },
    { "owner_password",    required_argument, 0, 'o' },
    { "pcr",               required_argument, 0, 'p' },
    { "remove",            no_argument,       0, 'R' },
    { "remove_password",   no_argument,       0, 'r' },
    { "resetlock",         no_argument,       0, 'z' },
    { "server",            required_argument, 0, 'S' },
    { "ssh",               no_argument,       0, 'i' },
    { "srk_password",      required_argument, 0, 's' },
    { "uuid",              required_argument, 0, 'u' },
    { "v1.1",              no_argument,       0, '1' },  
    { "v1.2",              no_argument,       0, '2' },
    { "verbose",           no_argument,       0, 'v' },
    { NULL, 0, 0, 0 },
};

static const char* options = "12A:bCDfhiK:k:ln:o:p:RrS:s:u:vz";

static void
help_brief(void)
{
    TKC_stderr("Try %s --help for more information.\n", PROGNAME);
}

static void
help_long(void)
{
    const char* help_message =
#include "help/help.h"
    ;
    TKC_stderr("%s", help_message);
}

int
main(int argc, char** argv)
{
    tkc_context_t* t = NULL;
    int c, option_index = 0, verbose = 0, ret = 0;
    uint64_t cmdmap = 0ULL;
    char* target_uuid = NULL;
    char* tss_server = NULL;
    tkc_pcrs_selected_t pcrs_selected;
    uint32_t open_flags = 0;
    uint32_t key_type = 0;
    uint32_t tss_version = TSS_TSPATTRIB_CONTEXT_VERSION_V1_1;

    char* owner_password = NULL;
    char* srk_password = NULL;
    char* keychain_password = NULL;
    char* key_password = NULL;
    char* new_password = NULL;
  
    if (argc <= 1) {
        help_brief();
        exit(1);
    }

    memset(&pcrs_selected, 0, sizeof(pcrs_selected));

    while ((c = getopt_long(argc, argv, options, long_options, &option_index))
           != -1) {
        switch (c) {
            case '1':
                tss_version = TSS_TSPATTRIB_CONTEXT_VERSION_V1_1;
                break;
            case '2':
                tss_version = TSS_TSPATTRIB_CONTEXT_VERSION_V1_2;
                break;
            case 'A':
                key_type = parse_key_type(optarg);
                if (key_type == TKC_KEY_TYPE_NONE) {
                    TKC_stderr("Invalid key type %s.\n", optarg);
                    exit(1);
                }
                cmdmap |= CMD_BITMAP(CMDBIT_ADD);
                break;
            case 'b':
                cmdmap |= CMD_BITMAP(CMDBIT_DUMPBLOB);
                break;
            case 'C':
                open_flags |= TKC_FLAG_CREATE;
                cmdmap |= CMD_BITMAP(CMDBIT_CREATE);
                break;
            case 'D':
                open_flags |= TKC_FLAG_DESTROY;
                cmdmap |= CMD_BITMAP(CMDBIT_DESTROY);
                break;
            case 'f':
                open_flags |= TKC_FLAG_FORCE;
                break;
            case 'h':
                help_long();
                exit(0);
            case 'k':
                key_password = optarg;
                break;
            case 'K':
                keychain_password = optarg;
                break;
            case 'i':
                cmdmap |= CMD_BITMAP(CMDBIT_SSH);
                break;
            case 'l':
                cmdmap |= CMD_BITMAP(CMDBIT_LIST);
                break;
            case 'n':
                new_password = optarg;
                cmdmap |= CMD_BITMAP(CMDBIT_CHANGEAUTH);
                cmdmap |= CMD_BITMAP(CMDBIT_NEWPASSWORD);
                break;
            case 'o':
                owner_password = optarg;
                break;
            case 'p': {
                errno = 0;
                char* end;
                uint64_t pcr = strtoull(optarg, &end, 0);
                if (errno || (end != optarg + strlen(optarg)) ||
                    (pcr >= TKC_MAX_PCRS)) {
                    TKC_stderr("Invalid PCR index %s.\n", optarg);
                    exit(1);
                }
                bit_set(pcrs_selected.bitmap, pcr);
                if ((uint32_t)pcr > pcrs_selected.highest) {
                    pcrs_selected.highest = (uint32_t)pcr;
                }
                pcrs_selected.count++;
                break;
            }
            case 'R':
                cmdmap |= CMD_BITMAP(CMDBIT_REMOVE);
                break;
            case 'r':
                cmdmap |= CMD_BITMAP(CMDBIT_REMOVEAUTH);
                break;
            case 'S':
                tss_server = optarg;
                break;
            case 's':
                srk_password = optarg;
                break;
            case 'u':
                target_uuid = optarg;
                cmdmap |= CMD_BITMAP(CMDBIT_UUID);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'z':
                open_flags |= (TKC_FLAG_NOKEYS | TKC_FLAG_NEEDOWNER);
                cmdmap |= CMD_BITMAP(CMDBIT_RESETLOCK);
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
        case CMD_ADD:
        case CMD_ADD_UUID:
        case CMD_CHANGEAUTH_UUID:
        case CMD_CREATE:
        case CMD_DESTROY:
        case CMD_DUMPBLOB_UUID:
        case CMD_LIST:
        case CMD_LIST_UUID:
        case CMD_REMOVE_UUID:
        case CMD_REMOVEAUTH_UUID:
        case CMD_RESETLOCK:
        case CMD_SSH_UUID:
            break;
        default:
            help_brief();
            exit(1);
    }

    t = tkc_open_context(tss_server,
                         owner_password, srk_password, keychain_password,
                         open_flags, tss_version);
    if (t == NULL) {
        exit(1);
    }

    switch (cmdmap) {
        case CMD_ADD:
        case CMD_ADD_UUID:
            ret = tkc_add_uuid(t, target_uuid, key_type,
                               (pcrs_selected.count == 0) ?
                                   NULL : &pcrs_selected,
                               key_password, tss_version);
            break;
        case CMD_CHANGEAUTH_UUID:
            ret = tkc_change_password_uuid(t, target_uuid,
                                           key_password, new_password);
            break;
        case CMD_CREATE:
            ret = 0;
            break;
        case CMD_DESTROY:
            ret = tkc_destroy(t);
            break;
        case CMD_DUMPBLOB_UUID:
            ret = tkc_dump_uuid(t, target_uuid);
            break;
        case CMD_LIST:
        case CMD_LIST_UUID:
            ret = tkc_list_uuid(t, target_uuid, verbose);
            break;
        case CMD_REMOVE_UUID:
            ret = tkc_remove_uuid(t, target_uuid);
            break;
        case CMD_REMOVEAUTH_UUID:
            ret = tkc_change_password_uuid(t, target_uuid, key_password, NULL);
            break;
        case CMD_RESETLOCK:
            ret = tkc_resetlock(t);
            break;
        case CMD_SSH_UUID:
            ret = tkc_ssh_uuid(t, target_uuid);
            break;
    }

    tkc_close_context(&t);

    exit(ret);
}
