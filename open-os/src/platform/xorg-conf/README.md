# "Xorg" configuration files

This repository contains configuration files that were originally for the X
Windowing System (a.k.a. X11 or Xorg). CrOS moved away from using X around 2015,
but these files are still used to configure input devices. Thankfully this
reduction in scope greatly reduces [their effect on life
satisfaction](https://xkcd.com/963/).

## Syntax

The syntax is now a subset of [Xorg's configuration syntax][man-xorg-conf], and
using anything outside of that subset will either cause an error or just be
ignored. Specifically, only `InputClass` sections are supported. Within those,
the following entries are supported:

* `Identifier`;
* some `Match` directives (`MatchProduct`, `MatchDevicePath`, `MatchUSBID`,
  `MatchIsPointer`, `MatchIsTouchpad`, and `MatchIsTouchscreen`);
* `Driver`, which is ignored; and
* `Option`, which is used to specify [gesture properties][gesture-props] instead
  of X11 input device options (though some old X11 options may be hanging
  around).

For more details, see the parsing code in Chromium's
[`GesturePropertyProvider`][gpp-xorg-parser].

[man-xorg-conf]: https://www.x.org/releases/current/doc/man/man5/xorg.conf.5.xhtml
[gesture-props]: https://chromium.googlesource.com/chromiumos/platform/gestures/+/HEAD/docs/gesture_properties.md
[gpp-xorg-parser]: https://source.chromium.org/chromium/chromium/src/+/HEAD:ui/events/ozone/evdev/libgestures_glue/gesture_property_provider.cc?q=GesturePropertyProvider::ParseXorgConfFile
