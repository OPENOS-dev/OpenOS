# `cbor`: Concise Binary Object Representation

This library is a partial implementation of the RFC 7049 Concise Binary Object
Representation standard.

The source code was fetched from
[`chromium/src`](https://chromium.googlesource.com/chromium/src/+/242df8b64d2a0ab5f057d1d4c76ea8537fdbb789)
in order to avoid code duplication.

The `cros/upstream/main` branch is a mirror of the `components/cbor` directory
from upstream. It is automatically updated to reflect the latest changes in
upstream.

## How to update the source

To pull in updates from `chromium/src`, do the following:

*   `git remote update`
*   `git checkout -b main cros/main`
*   `git merge cros/upstream/main`
    *   Expect merge conflicts, because of the difference in header paths.
    *   `OWNERS` should use the version from `main`.
    *   `BUILD.gn` should mostly use the version from `main`, unless the
        upstream changes the files to be built.
    *   The `#include` paths should use the version from `main` (without
        "components/"). This should be the majority of the merge conflicts.
    *   In the commit message of the merge, list the commits from upstream that
        are merged.
    *   If you need to do make changes to the merged commits (outside of
        conflicts), do that work in separate commits. For example, if you need
        to revert commits, use `git revert` after committing the merge commit.
        This preserves the history and makes it clear why a change is being
        reverted rather than quietly changing it in the merge commit.
    *   Check the changes introduced by your merge by doing a diff against the
        commit before the merge. The difference should be the same as the
        changes in the upstream.
*   Push the resulting merge commit with:

    ```bash
    (chroot) $ git push HEAD:refs/for/main
    ```
