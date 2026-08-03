# crdyshim

Pronounced CUR-dee-shim.

Crdyshim is a UEFI bootloader intended as an alternative to
[`shim`]. Its purpose is to load, verify, and run a more complicated
bootloader such as crdyboot. Since crdyshim is a first-stage bootloader
(meaning it is run directly by the firmware), it will need to be signed
by Microsoft for use on most PCs. By keeping crdyshim very simple, we
aim to make updates infrequent. This is important so that we don't have
to go through the full testing and signing process often.

This bootloader is in the same repo as crdyboot so that they can share
code and tests, but they are not inherently tied together other than
crdyshim hardcoding the filename of crdyboot.

## Boot flow

1. Get current SBAT revocations from the `SbatLevel` UEFI variable.
2. Optionally update the `SbatLevel`, if the embedded SBAT revocation
   data is newer.
3. Check the current executable's `.sbat` section against the current
   SBAT revocations; if crdyshim has been revoked, stop boot.
4. Load the next-stage executable from `<esp>/efi/boot/crdyboot<arch>.efi`.
5. Load the next-stage signature from `<esp>/efi/boot/crdyboot<arch>.sig`.
6. Use an embedded ECDSA public key and the signature to verify that the
   next-stage executable was signed by the expected key. Signature
   verification uses the [`p256`] crate.
7. If the signature is invalid, and secure boot is enabled, stop
   boot. If secure boot is not enabled, boot is allowed to continue with
   an invalid signature.
8. The executable data is measured into TPM PCR 4.
9. Check the next-stage executable's `.sbat` section against the current
   SBAT revocations; if it has been revoked, stop boot.
10. Apply relocations for the next-stage executable.
11. Apply NX memory attributes to the next-stage executable.
12. Launch the next-stage executable.

## Features

* Well documented and as simple as possible.
* Broad hardware support. Any amd64 machine with UEFI should be able to
  use crdyshim. This includes 32-bit UEFI environments.
* 100% Rust.

[`p256`]: https://docs.rs/p256/latest/p256/index.html
[`shim`]: https://github.com/rhboot/shim
