# About
baguette-motd provides a way to present Crostini users with a textual message printed into the Terminal app
(or any other terminal emulator launching a "login" shell in the Crostini container) upon startup.

The message is defined in the same script (baguette-motd.sh) that contains the presentation logic and will only
be presented a limited number of times before silencing itself. It also offers the user a way to silence it
earlier by executing a short shell command snippet appended to the message.

The baguette-motd concept is delivered as a versioned Debian package, to be automatically installed into the
Crostini container for both existing and new environments upon a ChromeOS release update.

# Updating
To specify the number of presentations before the message silences itself, set the difference between the
`COUNTER_MAX` and `COUNTER_INITIAL` constants in the `baguette-motd.sh` file accordingly.

To update the message and ensure that it will be shown for users that have already seen and silenced a
previous message, increment the package version in the `BUILD` file, update the `COUNTER_INITIAL`
and `COUNTER_MAX` constants such that they are both greater-than or equal to the previous version's
`COUNTER_MAX`, rebuild the package, and ensure it is included in the next container uprev.
