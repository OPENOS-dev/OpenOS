This is the profile bazel/alchemist uses when building host tools. It
should contain config settings that we don't want to affect the normal
CrOS chroot.

i.e., we want `USE=doc` in the CrOS chroot since it's an interactive
environment. We want `USE=-doc` for the bazel SDK because we want to
minimize the number and size of files.

It also imports the toolchain config so we can build the cross compilers
without using `crossdev`.
