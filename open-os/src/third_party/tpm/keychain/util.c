#include <tpm_keychain_common.h>
#include "tpm_keychain.h"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

static const char Base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';

static int
custom_b64_ntop(unsigned char const *src, size_t srclength,
                unsigned char *target, size_t targsize)
{
    size_t datalength = 0;
    unsigned char input[3];
    unsigned char output[4];
    int i;

    while (2 < srclength) {
        input[0] = *src++;
        input[1] = *src++;
        input[2] = *src++;
        srclength -= 3;

        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
        output[3] = input[2] & 0x3f;

        if (datalength + 4 > targsize) {
            return -1;
        }

        target[datalength++] = Base64[output[0]];
        target[datalength++] = Base64[output[1]];
        target[datalength++] = Base64[output[2]];
        target[datalength++] = Base64[output[3]];
    }
    
    if (0 != srclength) {

        input[0] = input[1] = input[2] = '\0';

        for (i = 0; i < srclength; i++) {
            input[i] = *src++;
        }
    
        output[0] = input[0] >> 2;
        output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
        output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);

        if (datalength + 4 > targsize) {
            return -1;
        }

        target[datalength++] = Base64[output[0]];
        target[datalength++] = Base64[output[1]];

        if (srclength == 1) {
            target[datalength++] = Pad64;
        } else {
            target[datalength++] = Base64[output[2]];
        }

        target[datalength++] = Pad64;
    }

    if (datalength >= targsize) {
        return -1;
    }

    target[datalength] = '\0';

    return datalength;
}

void
dump_rsa_ssh(RSA* rsa, char* uuid_string)
{
    unsigned char *p;
    unsigned char sshbuf[8192];
    unsigned char bigebuf[8192] = { 0 };
    unsigned int len = 0;
    unsigned int nb;
    int bige;
    int hbit = 1;
    BIGNUM* B;

    if (!rsa) {
        return;
    }

    p = sshbuf;

    len = strlen("ssh-rsa");
    p[0] = len >> 24;
    p[1] = len >> 16;
    p[2] = len >> 8;
    p[3] = len;
    p += 4;
    memcpy(p, "ssh-rsa", len);
    p += len;
    
    B = rsa->e;
    if (B->neg) {
        return;
    }

    if (BN_is_zero(B)) {
        p[0] = 0 >> 24;
        p[1] = 0 >> 16;
        p[2] = 0 >> 8;
        p[3] = 0;
        p += 4;
        goto GO_N;
    }

    if ((nb = BN_num_bytes(B) + 1) < 2) {
        return;
    }

    bigebuf[0] = '\0';
    bige = BN_bn2bin(B, bigebuf + 1);
    if (nb != (bige + 1)) {
        return;
    }
    if (*(bigebuf + 1) & 128) {
        hbit = 0;
    }
    p[0] = (nb - hbit) >> 24;
    p[1] = (nb - hbit) >> 16;
    p[2] = (nb - hbit) >> 8;
    p[3] = (nb - hbit);
    p += 4;
    memcpy(p, bigebuf + hbit, nb - hbit);
    p += nb - hbit;

GO_N:
    hbit = 1;
    B = rsa->n;
    bigebuf[0] = '\0';
    if (B->neg) {
        return;
    }

    if (BN_is_zero(B)) {
        p[0] = 0 >> 24;
        p[1] = 0 >> 16;
        p[2] = 0 >> 8;
        p[3] = 0;
        p += 4;
        goto OUT;
    }

    if ((nb = BN_num_bytes(B) + 1) < 2) {
        return;
    }

    bige = BN_bn2bin(B, bigebuf + 1);
    if (nb != (bige + 1)) {
        return;
    }
    if (*(bigebuf + 1) & 128) {
        hbit = 0;
    }
    p[0] = (nb - hbit) >> 24;
    p[1] = (nb - hbit) >> 16;
    p[2] = (nb - hbit) >> 8;
    p[3] = (nb - hbit);
    p += 4;
    memcpy(p, bigebuf + hbit, nb - hbit);
    p += nb - hbit;

OUT:
    len = (p - sshbuf) / (sizeof(unsigned char));
    if ((nb = custom_b64_ntop(sshbuf, len, bigebuf, (len << 1))) > 0) {
        TKC_stdout("# %s %s\n", TKC_SSH_UUID_TAG, uuid_string);
        TKC_stdout("%s %s\n", "ssh-rsa", bigebuf);
    }

    return;
}

const char* unparse_key_usage(uint32_t kusage)
{
    switch (kusage) {
        case TSS_KEYUSAGE_BIND:
            return "bind";
        case TSS_KEYUSAGE_IDENTITY:
            return "identity";
        case TSS_KEYUSAGE_LEGACY:
            return "bind,signing,ssh";
            break;
        case TSS_KEYUSAGE_SIGN:
            return "signing,ssh";
        case TSS_KEYUSAGE_MIGRATE:
            return "migrate";
        case TSS_KEYUSAGE_STORAGE:
            return "storage";
        case TSS_KEYUSAGE_AUTHCHANGE:
            return "authchange";
    }

    return "unknown";
}

uint32_t
parse_key_type(const char* ktype)
{
    if (ktype == NULL) {
        return TKC_KEY_TYPE_NONE;
    }

    if (!strcmp(ktype, "ssh")) {
        return TKC_KEY_TYPE_SSH;
    }

    if (!strcmp(ktype, "sign") || !strcmp(ktype, "signing")) {
        return TKC_KEY_TYPE_SIGNING;
    }

    if (!strcmp(ktype, "bind") || !strcmp(ktype, "binding")) {
        return TKC_KEY_TYPE_BIND;
    }

    if (!strcmp(ktype, "storage")) {
        return TKC_KEY_TYPE_STORAGE;
    }

    if (!strcmp(ktype, "legacy")) {
        return TKC_KEY_TYPE_LEGACY;
    }

    if (!strcmp(ktype, "default")) {
        return TKC_KEY_TYPE_LEGACY;
    }

    return TKC_KEY_TYPE_NONE;
}
