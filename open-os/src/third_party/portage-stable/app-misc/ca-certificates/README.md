app-misc/ca-certificates ebuild notes

This ebuild installs trusted CA certificates at `/etc/ssl/certs`, both
individually and as a concatenated PEM file at
`/etc/ssl/certs/ca-certificates.crt`. CA certificates are extracted
from the Mozilla NSS store. More details can be found at
the offical ChromeOS CA certificates [doc](https://www.chromium.org/chromium-os/developer-library/reference/infrastructure/ca-certs/#the-mozilla-nss-root-store)
