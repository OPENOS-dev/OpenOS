# CrOS Toolchains Overlay

This overlay holds the various CrOS toolchain packages.

There is a single `cross-*-linux-*` directory as all of our Linux toolchains use
the same underlying packages.  This isn't a strict requirement to keep in case
we ever want to change or tweak one target, but it makes management a little bit
easier.

If you need to update one of the generated configs, use the ./copy_crossdev.sh
script to copy the crossdev generated config from /etc/portage into this repo.
i.e.,
* `sudo crossdev --stable --show-fail-log --env 'FEATURES=splitdebug' -P --oneshot --overlays '/mnt/host/source/src/third_party/chromiumos-overlay /mnt/host/source/src/third_party/eclass-overlay /mnt/host/source/src/third_party/portage-stable' --ov-output /usr/local/portage/crossdev -t i686-cros-linux-gnu --ex-pkg sys-libs/libxcrypt --ex-pkg sys-libs/llvm-libunwind --ex-pkg sys-libs/libcxx --binutils '[stable]' --gcc '[stable]' --kernel '[stable]' --libc '[stable]' --ex-gdb --init-target`
* `./copy_crossdev.sh`

If you're adding a new package, `./copy_crossdev.sh` will do most of what you
want. You may need to add a symlink to the package under the triples you want
to support, though. e.g.,

`cd cross-x86_64-cros-linux-gnu && ln -s ../../chromiumos-overlay/dev-lang/rust`
