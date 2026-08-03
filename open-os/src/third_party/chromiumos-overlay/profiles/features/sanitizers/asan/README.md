# Chrome OS address sanitizer build profile

This directory contains the changes needed to build a board's packages with
[address sanitizer](https://www.chromium.org/chromium-os/how-tos-and-troubleshooting/llvm-clang-build).

## Building a board with address sanitizer

To build packages with address sanitizer (asan) for a board, create a profile in
the overlay and point its parent to the appropriate architecture. For example,
to add an asan profile for eve:

```bash
$ mkdir -p private-overlays/overlay-eve-private/profiles/asan
$ printf "../base\nchromiumos:features/sanitizers/asan/amd64\n" > \
    private-overlays/overlay-eve-private/profiles/asan/parent
```

If building a public board, create the profile in the public overlay.

```bash
$ mkdir -p overlays/overlay-eve/profiles/asan
$ printf "../base\nchromiumos:features/sanitizers/asan/amd64\n" > \
    overlays/overlay-eve/profiles/asan/parent
```

To build the eve board with asan profile, do:

```bash
$ setup_board --board=eve --profile=asan
$ build_packages --board=eve
```
