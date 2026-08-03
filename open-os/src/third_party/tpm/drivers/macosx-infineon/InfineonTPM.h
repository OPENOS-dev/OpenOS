// Copyright (C) 2006, Amit Singh <http://osxbook.com>
// Released under GPLv2.
// Borrows code from the Linux Infineon TPM driver.

#ifndef _INFINEON_TPM_H_
#define _INFINEON_TPM_H_

/* Timeout in msecs */
#define TPM_TIMEOUT 5

/* maximum number of WTX-packages */
#define TPM_MAX_WTX_PACKAGES 50

/* msleep-Time for WTX-packages */
#define TPM_WTX_MSLEEP_TIME 20

/* msleep-Time --> Interval to check status register */
#define TPM_MSLEEP_TIME 3

/* gives number of max. msleep()-calls before throwing timeout */
#define TPM_MAX_TRIES 5000

#define TPM_INFINEON_DEV_VEN_VALUE 0x15D1

/* TPM header definitions */
enum infineon_tpm_header {
    TPM_VL_VER                     = 0x01,
    TPM_VL_CHANNEL_CONTROL         = 0x07,
    TPM_VL_CHANNEL_PERSONALISATION = 0x0A,
    TPM_VL_CHANNEL_TPM             = 0x0B,
    TPM_VL_CONTROL                 = 0x00,
    TPM_INF_NAK                    = 0x15,
    TPM_CTRL_WTX                   = 0x10,
    TPM_CTRL_WTX_ABORT             = 0x18,
    TPM_CTRL_WTX_ABORT_ACK         = 0x18,
    TPM_CTRL_ERROR                 = 0x20,
    TPM_CTRL_CHAININGACK           = 0x40,
    TPM_CTRL_CHAINING              = 0x80,
    TPM_CTRL_DATA                  = 0x04,
    TPM_CTRL_DATA_CHA              = 0x84,
    TPM_CTRL_DATA_CHA_ACK          = 0xC4
};

enum infineon_tpm_register {
    WRFIFO = 0x00,
    RDFIFO = 0x01,
    STAT   = 0x02,
    CMD    = 0x03
};

enum infineon_tpm_command_bits {
    CMD_DIS  = 0x00,
    CMD_LP   = 0x01,
    CMD_RES  = 0x02,
    CMD_IRQC = 0x06
};

enum infineon_tpm_status_bits {
    STAT_XFE  = 0x00,
    STAT_LPA  = 0x01,
    STAT_FOK  = 0x02,
    STAT_TOK  = 0x03,
    STAT_IRQA = 0x06,
    STAT_RDA  = 0x07
};

/* some outgoing values */
enum infineon_tpm_values {
    CHIP_ID1              = 0x20,
    CHIP_ID2              = 0x21,
    TPM_DAR               = 0x30,
    RESET_LP_IRQC_DISABLE = 0x41,
    ENABLE_REGISTER_PAIR  = 0x55,
    IOLIMH                = 0x60,
    IOLIML                = 0x61,
    DISABLE_REGISTER_PAIR = 0xAA,
    IDVENL                = 0xF1,
    IDVENH                = 0xF2,
    IDPDL                 = 0xF3,
    IDPDH                 = 0xF4
};

#define TPM_BUFSIZE 2048

#endif /* _INFINEON_TPM_H_ */
