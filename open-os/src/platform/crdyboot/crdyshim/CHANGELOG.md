# Crdyshim Changelog

## 1.0.2

* Switch signature verification from Ed25519 to ECDSA.
  <http://crrev.com/c/7474111>, <http://crrev.com/c/7416135>,
  <http://crrev.com/c/7416136>
* Update SBAT revocations to 20250510. <http://crrev.com/c/7487010>
* Print verbose logs after a fatal error. <http://crrev.com/c/6204144>
* Log version on startup. <http://crrev.com/c/6696590>
* Make TPM logs more detailed. <http://crrev.com/c/5440295>
* Update to uefi-0.34. <http://crrev.com/c/5582506>,
  <http://crrev.com/c/5808421>, <http://crrev.com/c/5898543>,
  <http://crrev.com/c/6022537>, <http://crrev.com/c/6420861>
* Simplify file path code. <http://crrev.com/c/5582482>

## 1.0.1

* Update `uefi` and `uefi-services` deps. <http://crrev.com/c/5385531>
* Check that TPM is valid before using it. <http://crrev.com/c/5413794>, <http://crrev.com/c/5413795>
* Treat all TPM errors as non-fatal. <http://crrev.com/c/5413796>
* Change logging of non-fatal errors to the info level. <http://crrev.com/c/5413797>
* If secure boot is off, allow signature file to be missing. <http://crrev.com/c/5415295>
* Version bump. <http://crrev.com/c/5413798>

## 1.0.0

* Initial release. Everything up to (and including)
  [d3dfc4ff5c][d3dfc4ff5c].

[d3dfc4ff5c]: https://chromium.googlesource.com/chromiumos/platform/crdyboot/+/d3dfc4ff5c
