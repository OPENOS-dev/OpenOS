# Common PinWeaver Code

This directory contains reference PinWeaver code that can be used across implementation platforms.

It consists of:

- **PinWeaver reference** code:
    - `pinweaver.h` - PinWeaver embedded API definition
    - `pinweaver.c` - implementation
    - `pinweaver_eal.h` - API for Environment Abstraction Layer (EAL) used by PinWeaver
        - note that some types used in this API are platform-specific and are defined
            in `eal/**/pinweaver_eal_types.h`
    - `pinweaver_types.h` - header that is shared by PinWeaver implementation and
        PinWeaver clients that call it through platform-specific interface.
- **Environment Abstraction Layer (EAL)** implementations - in `eal/` folder
    - `eal/cr50` - implementation for [cr50](https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/cr50_stab)
        - `pinweaver_eal_types.h` - cr50-specific EAL API types
        - `pinweaver_eal.c` - cr50 implementation of EAL
    - `eal/tpm_storage` - implementation for platforms that use TPM as PinWeaver data storage
        - `pinweaver_eal_types.h` - TPM-storage-specific EAL API types
        - `pinweaver_eal_tpm.h` - additional EAL functions required by TPM storage
        - `pinweaver_eal_linux.c` - implementation of non-storage EAL methods for Linux case
        - `tpm_storage_stubs.c` - empty implementation of storage EAL methods
        - `tpm_storage.c` - implementation of storage EAL methods on top of TSS
        - `mini_trunks/` - **mini-TSS** (TPM client software stack) used by TPM storage implementation
            - created from [trunks TSS](https://chromium.googlesource.com/chromiumos/platform2/+/refs/heads/main/trunks/) used by ChromeOS reduced to the minimal required
                set of TPM commands and ported from C++ to C
            - relies on `pinweaver_eal.h` + `pinweaver_eal_tpm.h` EAL methods
            - TSS API is defined in `tss.h` + `*authorization_delegate.h`

A platform implementation that uses TPM storage EAL option needs to implement all EAL methods implemented
in `pinweaver_eal_linux.c` (or use it as-is, if Linux compatible).