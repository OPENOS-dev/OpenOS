These headers are used by packages that need Linux kernel headers.

In general one must be careful here. ChromeOS userspace programs need
to be able to run across a wide variety of kernel versions. This is
a reason to base the kernel headers on the "lowest supported kernel
version" and then add patches for extra things we need where we're
sure that the userspace using those patches is handling that fact that
not all of our kernels may support changes to the API.

TODO: working with these headers is a pain. At some point it would be
interesting to see if we should just have some sort of "git" repo and
manage it like the kernel, with UPSTREAM/BACKPORT/FROMGIT/FROMLIST
patches. Maybe this could even be another branch of the main kernel?

If you want to play with the patches and visualize them better, the
following may help. Note that patches currently need to be applied
with `patch -p1`, not `git am -3`. Ideally we should fix this.

---

```
cd ~/chromiumos/src/third_party/kernel/v6.6
git remote add upstream https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
git fetch upstream

git checkout v4.14
git branch -D fw-test
git checkout -b fw-test v4.14

for p in ~1/files/*.patch; do
  patch -p1 < $p;
  git add .
  git commit -m "$p"
done

for p in ~1/files/v4l2/*.patch; do
  patch -p1 < $p;
  git add .
  git commit -m "$p"
done
```

---

You can also see the result of applying patches with this (in the chroot):

```
cd ~/chromiumos/src/third_party/chromiumos-overlay/sys-kernel/linux-headers
ebuild-${BOARD} linux-headers-9999.ebuild prepare
```

...though this won't give you a "git" tree and thus it's harder to play
with the result.
