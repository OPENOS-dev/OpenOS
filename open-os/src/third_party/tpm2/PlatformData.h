// This file was extracted from the TCG Published
// Trusted Platform Module Library
// Part 4: Supporting Routines
// Family "2.0"
// Level 00 Revision 01.16
// October 30, 2014

#ifndef _PLATFORM_DATA_H_
#define _PLATFORM_DATA_H_
#include    "TpmBuildSwitches.h"
#include    "Implementation.h"
#include    "bool.h"
//
//     From Cancel.c Cancel flag. It is initialized as FALSE, which indicate the command is not being canceled
//
//
//     From Clock.c This variable records the time when _plat__ClockReset() is called. This mechanism allow
//     us to subtract the time when TPM is power off from the total time reported by clock() function
//
//
//     From LocalityPlat.c Locality of current command
//
//
//     From PPPlat.c Physical presence. It is initialized to FALSE
//
//
//     From Power
//
//
//     From Entropy.c
//

struct GlobalStructPlatform
{
    //
    //     From Cancel.c
    //
    BOOL                      s_isCanceled;
#define s_isCanceled global_plt_struct.s_isCanceled
    //
    //     From Clock.c
    //
    unsigned long long        s_initClock;
#define s_initClock global_plt_struct.s_initClock
    unsigned int              s_adjustRate;
#define s_adjustRate global_plt_struct.s_adjustRate
    //
    //     From LocalityPlat.c
    //
    unsigned char             s_locality;
#define s_locality global_plt_struct.s_locality
    //
    //     From Power.c
    //
    BOOL                      s_powerLost;
#define s_powerLost global_plt_struct.s_powerLost
    //
    //     From Entropy.c
    //
    uint32_t                  lastEntropy;
#define lastEntropy global_plt_struct.lastEntropy
    int                       firstValue;
#define firstValue global_plt_struct.firstValue
    //
    //     From PPPlat.c
    //
    BOOL   s_physicalPresence;
#define s_physicalPresence global_plt_struct.s_physicalPresence
};

LIB_IMPORT extern struct GlobalStructPlatform global_plt_struct;

#endif // _PLATFORM_DATA_H_
