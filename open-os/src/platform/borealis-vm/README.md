Borealis (Steam on ChromeOS, aka the Steam VM)
==============================================

_Self link: [go/cros-borealis](http://go/cros-borealis)_

The Borealis project provides first-party support for gaming on
CrOS devices using the Steam platform, developed by
Valve.  Steam is a cross-platform content delivery service specialising
in games and game-related technologies (benchmarks, engines, etc).

For more info, and team contacts, see [go/borealis](http://go/borealis).

Repository warning
------------------

The Chrome OS CQ only runs PRESUBMIT.cfg for this repository; it doesn't go
through the full CQ.  Don't put code in here that is built by an ebuild
(src/platform2/vm_tools might be more appropriate) as it will not be tested.

The Borealis Docker image is built regularly and uprevved with PUpr into the
borealis-dlc ebuild, which is then tested by CQ builders for Borealis-capable
boards.

See [go/steam-shield](https://goto.google.com/steam-shield) for links to the
rootfs builder, the PUpr generator, and borealis-cq.

Links
-----

[Internal team site](https://goto.google.com/borealis)
[Care and feeding](https://goto.google.com/borealis-care)
