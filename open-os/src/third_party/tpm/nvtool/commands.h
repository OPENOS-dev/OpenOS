// Copyright (c) 2009,2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_NVTOOL_COMMANDS_H_
#define _TPM_NVTOOL_COMMANDS_H_

enum {
    CMDBIT_TNV_DATA,
    CMDBIT_TNV_DEFINE,
    CMDBIT_TNV_FILE,
    CMDBIT_TNV_INDEX,
    CMDBIT_TNV_LIST,
    CMDBIT_TNV_READ,
    CMDBIT_TNV_RELEASE,
    CMDBIT_TNV_WRITE,
    CMDBIT_TNV_WRITEZERO,
};

#define CMD_BITMAP(cmd) (1ULL << cmd)

enum {
    CMD_TNV_DEFINE     = (CMD_BITMAP(CMDBIT_TNV_DEFINE)    | \
                          CMD_BITMAP(CMDBIT_TNV_INDEX)),
    CMD_TNV_LIST       = (CMD_BITMAP(CMDBIT_TNV_LIST)),
    CMD_TNV_LIST_INDEX = (CMD_TNV_LIST | CMD_BITMAP(CMDBIT_TNV_INDEX)),
    CMD_TNV_READ       = (CMD_BITMAP(CMDBIT_TNV_READ)      | \
                          CMD_BITMAP(CMDBIT_TNV_INDEX)),
    CMD_TNV_RELEASE    = (CMD_BITMAP(CMDBIT_TNV_RELEASE)   | \
                          CMD_BITMAP(CMDBIT_TNV_INDEX)),
    CMD_TNV_WRITE      = (CMD_BITMAP(CMDBIT_TNV_WRITE)     | \
                          CMD_BITMAP(CMDBIT_TNV_INDEX)     | \
                          CMD_BITMAP(CMDBIT_TNV_DATA)),
    CMD_TNV_WRITE_FILE = (CMD_BITMAP(CMDBIT_TNV_WRITE)     | \
                          CMD_BITMAP(CMDBIT_TNV_INDEX)     | \
                          CMD_BITMAP(CMDBIT_TNV_FILE)),
    CMD_TNV_WRITEZERO  = (CMD_BITMAP(CMDBIT_TNV_WRITEZERO) | \
                          CMD_BITMAP(CMDBIT_TNV_INDEX)),
};

#endif // _TPM_NVTOOL_COMMANDS_H_
