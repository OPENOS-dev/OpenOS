This feature profile enables the gcov USE flag for every chromeos-kernel package.
It is intended to be used for CrOS automated kernel test coverage.

To add a coverage kernel profile to an existing overlay, take the following steps:
1) Create a new subdirectory within the given overlay's "profiles" directory.
2) Add a "parent" file to the new subdirectory with the below tow lines:
  ```
  ../base
  chromiumos:features/kernel/coverage
  ```

  The first line specifies that the new profile will inherit from the overlay's
  base profile. The second line will mix in this coverage profile which
  enables the gcov USE flags for the chromeos-kernel package, independent of which kernel version the overlay uses.
