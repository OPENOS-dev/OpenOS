# `redaction_tool`: PII Redaction Tool

A redaction_tool redacts PII using a wide set of RegEx expressions.

This code is a mirror of
https://source.chromium.org/chromium/chromium/src/+/main:components/feedback/redaction_tool/

Changes should be made in chromium code base only and then be upreved here.

## How to update the source

To pull in updates from `chromium/src`, do the following:

*   `git remote update`
*   `git checkout -b main cros/main`
*   `git merge cros/upstream/main`
    *   `BUILD.gn` should mostly use the version from `main`, unless the
        upstream changes the files to be built.
    *   The #include paths should use the version from main
        (without "components/feedback").
        This should be the majority of the merge conflicts.
    *   In the commit message of the merge, list the commits from upstream that
        are merged.
    *   If you need to make changes to the merged commits (outside of
        conflicts), do that work in separate commits. For example, if you need
        to revert commits, use `git revert` after committing the merge commit.
        This preserves the history and makes it clear why a change is being
        reverted rather than quietly changing it in the merge commit.
    *   Check the changes introduced by your merge by doing a diff against the
        commit before the merge. The difference should be the same as the
        changes in the upstream.
*   Push the resulting merge commit for review with:
    *   `git push HEAD:refs/for/main`
