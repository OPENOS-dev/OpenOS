// This file was extracted from the TCG Published
// Trusted Platform Module Library
// Part 4: Supporting Routines
// Family "2.0"
// Level 00 Revision 01.16
// October 30, 2014

#define GLOBAL_C
#include "InternalRoutines.h"

const UINT16              g_rcIndex[15] = {TPM_RC_1,       TPM_RC_2,    TPM_RC_3, TPM_RC_4,
                                          TPM_RC_5,       TPM_RC_6,    TPM_RC_7, TPM_RC_8,
                                          TPM_RC_9,       TPM_RC_A,    TPM_RC_B, TPM_RC_C,
                                          TPM_RC_D,       TPM_RC_E,    TPM_RC_F
                                       };

struct GlobalStruct global_struct __attribute__((section(".bss.Tpm2_common")));
