Control USE flags of packages regarding factory support.

'factory-branch' disables features that are not required in factory branch test
image.

To add this profile to an existing overlay in a factory branch:
1.  Create "profiles/base/parent" if it does not exist.
2.  Append "chromiumos:features/factory/factory-branch" as the last line of
    "profiles/base/parent".
