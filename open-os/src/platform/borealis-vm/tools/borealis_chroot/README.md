# go/borealis_chroot

Run the Borealis rootfs in a chroot on your device.  A bit like Crouton, except
using Sommelier and Xwayland for graphics rather than taking over the display
and running an X server.

To run `borealis_chroot`:

- Start in `src/platform/borealis`, and set your `$DUT` variable:
  ~~~
  cd ~/chromiumos/src/platform/borealis
  DUT=<your dut's SSH hostname>
  ~~~
- Option 1: Download the latest chroot rootfs built by our automated builders: go/borealis-chroot-prebuilts.
Extract the tarball, discard `vm_kernel`, and rename the rootfs to `borealis_chroot_rootfs.img`.
- Option 2: Skip if you chose option 1. Build and convert the `borealis_chroot` rootfs stage.
If you have not built Borealis before or your last Borealis build is stale, do not include `--skip-termina`.
  ~~~
  tools/build_full.py \
    --variant=chroot \
    --image-name=borealis_chroot \
    --skip-termina
  tools/convert_docker_image.py \
    --docker-image=borealis_chroot \
    --output=borealis_chroot_rootfs.img
  ~~~
- Copy `borealis_chroot_rootfs.img` to your device:
  ~~~
  ssh $DUT -- mkdir -p /mnt/stateful_partition/img_borealis/
  rsync --progress --compress borealis_chroot_rootfs.img $DUT:/mnt/stateful_partition/img_borealis/
  ~~~
- (ChromeOS) Run `borealis_chroot` using the helper script in this folder:
  ~~~
  tools/borealis_chroot/run_on_dut.sh $DUT
  ~~~
- (Reference Linux, e.g. Ubuntu) Run `borealis_chroot` using the helper script in this folder:
  ~~~
  tools/borealis_chroot/run_on_dut.sh -l $HOMEDIR $DUT
  ~~~

You should now have a shell inside the chroot, with a working graphics/sound environment.

Start Steam with: `steam -no-cef-sandbox`

## Sharing your Borealis stateful and external disk

By default, `borealis_chroot` shares nothing with the Borealis installation on
the same device, which lets you run Borealis and `borealis_chroot` at the same
time.  If you're okay with not being able to run both at once, you can
optionally mount your Borealis stateful image like this, and just
download/update each game once:

~~~
tools/borealis_chroot/run_on_dut.sh -m $DUT
~~~

If you also want to mount your external disk, this requires a custom kernel to
support btrfs:

~~~
cros_sdk
for kernel in v5.4 v5.10; do
  KERNEL_PATH="$HOME/chromiumos/src/third_party/kernel/$kernel"
  CONF="$KERNEL_PATH/chromeos/config/chromeos/base.config"
  echo "Kernel $kernel in $KERNEL_PATH"
  if ! grep CONFIG_BTRFS_FS $CONF; then
    echo -e "CONFIG_BTRFS_FS=y\nCONFIG_BTRFS_FS_POSIX_ACL=y" >> $CONF
    echo "Added BTRFS config to $CONF"
  fi
done

# Volteer
cros_workon --board=volteer start chromeos-kernel-5_4 && \
emerge-volteer chromeos-kernel-5_4 && \
~/chromiumos/src/scripts/update_kernel.sh --remote=myvolteer

# Brya
cros_workon --board=brya start chromeos-kernel-5_10 && \
emerge-brya chromeos-kernel-5_10 && \
~/chromiumos/src/scripts/update_kernel.sh --remote=mybrya
~~~

Once your kernel is updated, you can mount your external disk image (after
starting borealis_chroot) like this:

~~~
tools/borealis_chroot/run_on_dut.sh -m \
    -x /media/removable/extssd/borealis_drive2 $DUT
~~~

## Troubleshooting

If you don't see a bash prompt, check that it's not obscured by xkbcomp errors
-- try tapping ENTER.

If it freezes after printing `glxinfo` output, just `Ctrl-C` out and rerun the
script; it should work the second time.
