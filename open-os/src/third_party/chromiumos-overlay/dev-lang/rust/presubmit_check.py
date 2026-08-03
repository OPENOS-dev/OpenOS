#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Does presubmit checks for Rust-like changes.

This is intended to be used by `repo`. For more information, please see
https://chromium.googlesource.com/chromiumos/repohooks/+/refs/heads/main/README.md.

Enforces that all Rust toolchain packages, as well as the Rust virtual, are at
the same ${PVR}. This does not apply to rust-bootstrap.
"""

import dataclasses
import os
import re
import subprocess
import sys
import textwrap
from typing import List, Optional


_ALL_RUST_DIRS = ("virtual/rust/", "dev-lang/rust/", "dev-lang/rust-host/")
_CROS_RUSTC_ECLASS = "eclass/cros-rustc.eclass"

_Complaint = str


def _has_cros_rustc_version_change(sha: str, modified_files: List[str]) -> bool:
    if _CROS_RUSTC_ECLASS not in modified_files:
        return False

    diff_stdout = subprocess.run(
        ("git", "diff", f"{sha}~..{sha}", _CROS_RUSTC_ECLASS),
        check=True,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        encoding="utf-8",
    ).stdout

    # This variable is documented as being matched on by other scripts, so
    # regex'ing it here is fine.
    stable_version_re = re.compile(
        r'^[+-]RUSTC_STABLE_VERSION="([^"]+)"', re.MULTILINE
    )
    matches = list(stable_version_re.finditer(diff_stdout))
    if not matches:
        return False

    # If there aren't exactly zero or two matches, it's likely someone's
    # removing this variable, or added a new instance? Can't reasonably handle
    # either.
    if len(matches) != 2:
        raise ValueError(
            f"Expected precisely zero or two matches for {stable_version_re}"
            f"in cros-rustc's diff; found {len(matches)}."
        )

    # (No effort is put into verifying `old == line.startswith('-')`, since
    # inequality is symmetric.)
    old_value = matches[0].group(1)
    new_value = matches[1].group(1)
    return old_value != new_value


@dataclasses.dataclass(frozen=True)
class CommitChanges:
    """Contains information about the modifications performed by a commit."""

    # A list of files that were only modified in-place; not deleted, created,
    # renamed, etc.
    files_only_modified: List[str]
    # All files changed by a commit that aren't in `files_only_modified`. Note
    # that renames will have an entry for _both_ the rename-from and rename-to
    # locations.
    all_other_changed_files: List[str]
    # If True, this change includes a modification to the cros rustc version.
    # Modifying this will change the version of both dev-lang/rust and
    # dev-lang/rust-host.
    includes_cros_rustc_version_bump: bool

    @property
    def all_changed_files(self) -> List[str]:
        return self.files_only_modified + self.all_other_changed_files

    @classmethod
    def from_commit(cls, sha: str) -> "CommitChanges":
        # This command outputs a line-separated list of files changed by a
        # commit, in the form:
        # operation     file_name (new_file_name)?
        #
        # e.g.,
        # M      dev-lang/rust/presubmit_check.py
        # R100   dev-lang/rust/foo dev-lang/rust/foo2
        output = subprocess.check_output(
            [
                "git",
                "show",
                "--name-status",
                sha,
                "--format=",
                # Only report Added, Deleted, Copied, Modified, Renamed, and
                # Type-Changed files. Other types seem to have git-inter
                "--diff-filter=ADMRT",
            ],
            encoding="utf-8",
        )

        output_re = re.compile(r"^([ADMRT])\d*\s+(\S+)(?:\s+(\S+))?$")
        all_other_changed_files = []
        files_only_modified = []
        for line in output.splitlines():
            line = line.strip()
            if not line:
                continue
            m = output_re.match(line)
            assert m, f"Unexpectedly failed to match {line!r} with {output_re}"
            mod_type, file1, maybe_file2 = m.groups()
            if mod_type == "R":
                assert maybe_file2, f"No second file for a rename? Line: {line}"
                all_other_changed_files += (file1, maybe_file2)
                continue
            assert not maybe_file2, (
                f"Second file unexpectedly present for {mod_type} operation. "
                f"Line: {line}"
            )
            if mod_type == "M":
                files_only_modified.append(file1)
            else:
                all_other_changed_files.append(file1)
        return cls(
            all_other_changed_files=all_other_changed_files,
            files_only_modified=files_only_modified,
            includes_cros_rustc_version_bump=_has_cros_rustc_version_change(
                sha,
                modified_files=files_only_modified + all_other_changed_files,
            ),
        )


def _will_change_to_file_cause_revbump(
    file: str, package_directory: str
) -> bool:
    """Returns whether changes to `file` cause `package` to be revbumped."""
    file_in_package = os.path.relpath(file, package_directory)
    if file_in_package.startswith("../"):
        return False
    return (
        file_in_package == "Manifest"
        or file_in_package.startswith("files/")
        or file_in_package.endswith("-9999.ebuild")
    )


def _ensure_cros_rustc_changes_have_revbumps(
    changes: CommitChanges,
) -> Optional[_Complaint]:
    all_changed_files = changes.all_changed_files
    if not any(x == _CROS_RUSTC_ECLASS for x in all_changed_files):
        return

    # Changes to the Rust version always cause both packages to be revbumped.
    if changes.includes_cros_rustc_version_bump:
        return None

    for file in all_changed_files:
        if _will_change_to_file_cause_revbump(
            file, "dev-lang/rust/"
        ) or _will_change_to_file_cause_revbump(file, "dev-lang/rust-host/"):
            # Just `return None` here, since something, somewhere will
            # cause a revbump to a Rust package. The 'lockstep' check later
            # ensures that all other packages are revbumped properly.
            return None

    return textwrap.dedent(
        f"""\
        {_CROS_RUSTC_ECLASS} was changed, but no corresponding cros-rustc users
        were changed in a way that will cause them to be uprevved by Annealing.
        If this is intended, pass `--ignore-hooks` or `--no-verify` to `repo`.
        Otherwise, please update `files/revision_bump` in dev-lang/rust and
        dev-lang/rust-host.
        """
    )


def _ensure_rust_and_rust_host_are_in_lockstep(
    changes: CommitChanges,
) -> Optional[_Complaint]:
    # Changes to the Rust version always cause both packages to be revbumped.
    if changes.includes_cros_rustc_version_bump:
        return None

    has_rust_change = False
    has_rust_host_change = False
    for file in changes.all_changed_files:
        if _will_change_to_file_cause_revbump(file, "dev-lang/rust/"):
            has_rust_change = True
            if has_rust_host_change:
                break
        elif _will_change_to_file_cause_revbump(file, "dev-lang/rust-host/"):
            has_rust_host_change = True
            if has_rust_change:
                break

    if has_rust_change == has_rust_host_change:
        return None

    if has_rust_change:
        package_with_changes = "rust"
        package_missing_changes = "rust-host"
    else:
        package_missing_changes = "rust"
        package_with_changes = "rust-host"

    return textwrap.dedent(
        f"""\
        Change(s) in dev-lang/{package_with_changes} detected, but no change is
        in dev-lang/{package_missing_changes}. This can lead to rust and
        rust-host becoming desynced, which will break your CQ+1 run. If you'd
        like to force an uprev of dev-lang/{package_missing_changes}, you can
        update dev-lang/{package_missing_changes}/files/revision_bump.
        """
    )


def _ensure_no_virtual_rust_revbumps(
    changes: CommitChanges,
) -> Optional[_Complaint]:
    """Complains if the commit modifies ebuild names in virtual/rust."""
    virtual_rust_ebuild = re.compile(r"virtual/rust/.*\.ebuild")
    # Ignore simple modifications of virtual/rust. It's OK to change it in ways
    # that don't involve bringing it out of sync with the other Rust packages.
    has_potential_revbumps = any(
        virtual_rust_ebuild.match(x) for x in changes.all_other_changed_files
    )
    if not has_potential_revbumps:
        return None

    return textwrap.dedent(
        """\
        Creations/deletions/renames to ebuilds in virtual/rust detected. The
        SDK builder maintains this file; it should not be uprevved by humans.
        Bypass this check if you're certain you want to do this.
        """
    )


def _is_rusty_file(file_path: str) -> bool:
    """Returns whether the param is in a rust dir we should check."""
    return file_path == _CROS_RUSTC_ECLASS or any(
        file_path.startswith(x) for x in _ALL_RUST_DIRS
    )


def main():
    """Main function."""
    presubmit_files = os.environ.get("PRESUBMIT_FILES")
    if presubmit_files is None:
        sys.exit("Need a value for PRESUBMIT_FILES")

    # Skip this for CLs that don't touch Rust. It's faster, and if this is
    # broken at ToT somehow, we don't want to halt all uploads on unrelated
    # CLs.
    presubmit_files = [x.strip() for x in presubmit_files.splitlines()]
    if not any(_is_rusty_file(x) for x in presubmit_files):
        return

    presubmit_commit = os.environ.get("PRESUBMIT_COMMIT")
    if presubmit_commit is None:
        sys.exit("Need a value for PRESUBMIT_COMMIT")

    changes = CommitChanges.from_commit(presubmit_commit)
    checks = (
        _ensure_no_virtual_rust_revbumps,
        _ensure_rust_and_rust_host_are_in_lockstep,
        _ensure_cros_rustc_changes_have_revbumps,
    )

    had_complaint = False
    for check in checks:
        if c := check(changes):
            had_complaint = True
            print(f"error: {c}", file=sys.stderr)
    sys.exit(1 if had_complaint else 0)


if __name__ == "__main__":
    main()
