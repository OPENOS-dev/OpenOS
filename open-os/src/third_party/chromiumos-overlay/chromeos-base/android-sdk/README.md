This package provides Android SDK to build the test apks used by Tast tests.

# How to generate licenses/AOSP-SDK

files/combine_notice.py combines the license files in the downloaded archives
into one file.

```shell
$ file/combine_notice.py --sdk_dir ${PATH to unpacked SDK directory} --output ../../licenses/AOSP-SDK
```

After generating the AOSP-SDK file, please replace the common license text with
`SPDX-License-Identifier` tag to reduce the size of the license file.
The licenses listed in the ebuild file are replaced with the tag.
