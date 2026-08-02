# libtextclassifier

A library for on-device text classification (address, telephone number,
emails etc.).

# Usage in Chrome OS

To use libtextclassifier, one also needs to install a flatbuffer model defined
in "model.fbs". For simplicity, this model file will be installed in the
[ebuild of ml-service][ml-9999-ebuild].

And libtextclassifier currently [depends on][tclib-depend-chrome-icu] the
[chrome-icu][chrome-icu-ebuild-folder] package which requires explicitly
initializing the icu data, just like [chrome][chrome-init-icu].

The [ml-service] is currently the only user of this library. If another library
in Chrome OS also wants to use it, please let the owners of [ml-service] and
libtextclassifier know because then,

1. we need to make libtextclassifier into a shared library to save space;
2. we may also need to move the installation of libtextclassifier's model file
into libtextclassifier's ebuild.


[chrome-icu-ebuild-folder]: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/chromeos-base/chrome-icu/
[chrome-init-icu]: https://source.chromium.org/chromium/chromium/src/+/master:base/i18n/icu_util.cc;l=234;drc=928bcad29884064f83bf05ecadf82369f8b4622f?originalUrl=https:%2F%2Fcs.chromium.org%2F
[ml-9999-ebuild]: https://source.chromium.org/chromiumos/chromiumos/codesearch/+/master:src/third_party/chromiumos-overlay/chromeos-base/ml/ml-9999.ebuild
[ml-service]: https://source.chromium.org/chromiumos/chromiumos/codesearch/+/master:src/platform2/ml/;l=1?q=ml%2F&sq=&ss=chromiumos
[tclib-depend-chrome-icu]: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/dev-libs/libtextclassifier/libtextclassifier-9999.ebuild#24
