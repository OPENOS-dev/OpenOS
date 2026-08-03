#!/usr/bin/env python3
# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generates an html of all the licenses used by Borealis.

Typical usage:
tools/build_full.py --package-list packages.txt --licenses-tar licenses.tar
tar xf licenses.tar
tools/generate_license_html.py packages.txt licenses -o borealis-credits.html
"""

import argparse
import os
import re
import string
from typing import List


# Modified versions of the credits from chromite.
TEMPLATE_DIR = os.path.join(os.path.dirname(__file__), "..", "templates")
CREDITS_TEMPLATE = os.path.join(TEMPLATE_DIR, "about_credits.tmpl")
ENTRY_TEMPLATE = os.path.join(TEMPLATE_DIR, "about_credits_entry.tmpl")
SHARED_TEMPLATE = os.path.join(
    TEMPLATE_DIR, "about_credits_shared_license_entry.tmpl"
)

# These packages reference licenses formatted like common licenses
# (i.e. without the "custom:" prefix), but actually provide them as
# custom licenses in /usr/share/licenses/$pkgname.  Treat them as
# custom licenses.
CUSTOM_LICENSE_ALLOWLIST = {
    # OFL-1.1 license provided in /usr/share/licenses/adobe-source-code-pro-fonts
    "adobe-source-code-pro-fonts": ["OFL-1.1"],
    # OFL-1.1 license provided in /usr/share/licenses/adwaita-fonts
    "adwaita-fonts": ["OFL-1.1"],
    # LLVM-exception license provided in /usr/share/licenses/libcups
    "libcups": ["LLVM-exception"],
    # AFL-2.1 license provided in /usr/share/licenses/dbus/COPYING
    "dbus": ["AFL-2.1"],
    "lib32-dbus": ["AFL-2.1"],
    # Various licenses provided in /usr/share/licenses/fontconfig
    "fontconfig": ["Unicode-DFS-2016", "HPND"],
    # Various licenses provided in /usr/share/licenses/freetype2
    "freetype2": ["FTL", "MIT-Modern-Variant"],
    # Exception provided in /usr/share/licenses/gcc-libs/RUNTIME.LIBRARY.EXCEPTION
    "gcc": ["GPL-3.0-with-GCC-exception"],
    "gcc-libs": ["GPL-3.0-with-GCC-exception"],
    "lib32-gcc-libs": ["GPL-3.0-with-GCC-exception"],
    # X11 license provided in /usr/share/licenses/libgcrypt
    "libgcrypt": ["X11"],
    # MIT, SGI-B-2.0 licenses provided in /usr/share/licenses/glu
    "glu": ["SGI-B-2.0"],
    # UnicodeData license provided in /usr/share/licenses/gnupg/Unicode-TOU.txt
    # BSD-4-Clause license provided in /usr/share/licenses/gnupg/BSD-4-Clause.txt
    "gnupg": ["Unicode-TOU", "BSD-4-Clause"],
    # MIT-open-group license provided in /usr/share/licenses/libice
    "libice": ["MIT-open-group"],
    # NAIST-2003 license provided in /usr/share/licenses/icu
    "icu": ["NAIST-2003"],
    # NAIST-2003 license provided in /usr/share/licenses/lib32-icu
    "lib32-icu": ["NAIST-2003"],
    # IJG license provided in /usr/share/licenses/libjpeg-turbo/README.ijg
    "libjpeg-turbo": ["IJG"],
    # 0BSD license provided in /usr/share/licenses/lilv/0BSD.txt
    "lilv": ["0BSD"],
    # LLVM-exception license provided in /usr/share/licenses/llvm
    "llvm": ["LLVM-exception"],
    # LLVM-exception license provided in /usr/share/licenses/llvm-libs
    "llvm-libs": ["LLVM-exception"],
    # LLVM-exception license provided in /usr/share/licenses/lib32-llvm
    "lib32-llvm": ["LLVM-exception"],
    # LLVM-exception license provided in /usr/share/licenses/lib32-llvm-libs
    "lib32-llvm-libs": ["LLVM-exception"],
    # MIT-open-group license provided in /usr/share/licenses/ncurses
    "ncurses": ["MIT-open-group"],
    # PCRE2-exception license provided in /usr/share/licenses/pcre2
    "pcre2": ["PCRE2-exception"],
    # PCRE2-exception license provided in /usr/share/licenses/lib32-pcre2
    "lib32-pcre2": ["PCRE2-exception"],
    # MIT-CMU license provided in /usr/share/licenses/python-pillow
    "python-pillow": ["MIT-CMU"],
    # Python-2.0.1 license provided in /usr/share/licenses/python-typing_extensions
    "python-typing_extensions": ["Python-2.0.1"],
    # BSD-3-Clause-Attribution license provided in /usr/share/licenses/libsasl
    "libsasl": ["BSD-3-Clause-Attribution"],
    # 0BSD license provided in /usr/share/licenses/serd/0BSD.txt
    "serd": ["0BSD"],
    # MIT-open-group license provided in /usr/share/licenses/libsm
    "libsm": ["MIT-open-group"],
    # BSD-2-Clause-Patent license provided in /usr/share/licenses/libsysprof-capture
    "libsysprof-capture": ["BSD-2-Clause-Patent"],
    # MIT-0 license provided in /usr/share/licenses/systemd
    "systemd": ["MIT-0"],
    # SISSL license provided in /usr/share/licenses/libtirpc
    "libtirpc": ["SISSL"],
    # X11 license provided in /usr/share/licenses/lib32-libx11
    "lib32-libx11": ["X11"],
    # X11 license provided in /usr/share/licenses/libx11
    "libx11": ["X11"],
    # MIT-open-group license provided in /usr/share/licenses/libxau
    "libxau": ["MIT-open-group"],
    # Various licenses provided in /usr/share/licenses/libxaw
    "libxaw": [
        "HPND",
        "HPND-sell-variant",
        "MIT-open-group",
        "NTP",
        "SMLNJ",
        "X11",
    ],
    # X11 license provided in /usr/share/licenses/libxcb
    "libxcb": ["X11"],
    # X11-distribute-modifications-variant license provided in /usr/share/licenses/xcb-proto
    "xcb-proto": ["X11-distribute-modifications-variant"],
    # X11-distribute-modifications-variant license provided in /usr/share/licenses/xcb-util-keysyms
    "xcb-util-keysyms": ["X11-distribute-modifications-variant"],
    # X11-distribute-modifications-variant license provided in /usr/share/licenses/xcb-util-wm
    "xcb-util-wm": ["X11-distribute-modifications-variant"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxcomposite
    "libxcomposite": ["HPND-sell-variant"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxcvt
    "libxcvt": ["HPND-sell-variant"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxcursor
    "libxcursor": ["HPND-sell-variant"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxdamage
    "libxdamage": ["HPND-sell-variant"],
    # MIT-open-group license provided in /usr/share/licenses/libxdmcp
    "libxdmcp": ["MIT-open-group"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxfixes
    "libxfixes": ["HPND-sell-variant"],
    # Various licenses provided in /usr/share/licenses/libxfont2
    "libxfont2": ["HPND-sell-variant", "MIT-open-group", "SMLNJ", "X11"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxft
    "libxft": ["HPND-sell-variant"],
    # Various licenses provided in /usr/share/licenses/libxi
    "libxi": ["MIT-open-group", "SMLNJ"],
    # Various licenses provided in /usr/share/licenses/libxinerama
    "libxinerama": ["MIT-open-group", "X11"],
    # Various licenses provided in /usr/share/licenses/libxmu
    "libxmu": [
        "MIT-open-group",
        "SMLNJ",
        "X11",
        "X11-distribute-modifications-variant",
    ],
    # Various licenses provided in /usr/share/licenses/xorg-server-common
    "xorg-server-common": [
        "HPND",
        "HPND-sell-variant",
        "ICU",
        "MIT-open-group",
        "NTP",
        "SMLNJ",
        "SGI-B-2.0",
        "X11",
        "X11-distribute-modifications-variant",
    ],
    # Various licenses provided in /usr/share/licenses/xorgproto
    "xorgproto": [
        "HPND",
        "HPND-sell-variant",
        "ICU",
        "MIT-open-group",
        "SMLNJ",
        "SGI-B-2.0",
        "X11",
        "X11-distribute-modifications-variant",
    ],
    # xorg-xprop license provided in /usr/share/licenses/xorg-xprop
    "xorg-xprop": ["MIT-open-group"],
    # xorg-xrandr license provided in /usr/share/licenses/xorg-xrandr
    "xorg-xrandr": ["HPND-sell-variant"],
    # X11-distribute-modifications-variant license provided in /usr/share/licenses/libxpm
    "libxpm": ["X11-distribute-modifications-variant"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxrender
    "libxrender": ["HPND-sell-variant"],
    # X11 license provided in /usr/share/licenses/libxres
    "libxres": ["X11"],
    # HPND-sell-variant license provided in /usr/share/licenses/libxshmfence
    "libxshmfence": ["HPND-sell-variant"],
    # X11 license provided in /usr/share/licenses/libxss
    "libxss": ["X11"],
    # Various licenses provided in /usr/share/licenses/libxt
    "libxt": ["HPND-sell-variant", "MIT-open-group", "SMLNJ", "X11"],
    # Various licenses provided in /usr/share/licenses/xtrans
    "xtrans": ["HPND", "HPND-sell-variant", "MIT-open-group", "X11"],
    # Various licenses provided in /usr/share/licenses/libxtst
    "libxtst": [
        "HPND-doc",
        "HPND-doc-sell",
        "HPND-sell-variant",
        "MIT-open-group",
        "X11",
    ],
    # Various licenses provided in /usr/share/licenses/libxv
    "libxv": ["HPND-sell-variant", "SMLNJ"],
    # X11-distribute-modifications-variant license provided in /usr/share/licenses/libxxf86vm
    "libxxf86vm": ["X11-distribute-modifications-variant"],
    # 0BSD license provided in /usr/share/licenses/zix/0BSD.txt
    "zix": ["0BSD"],
}

# These packages reference build-time licenses that we can ignore.
IGNORABLE_LICENSE_ALLOWLIST = {}

# Mappings for legacy licenses that used to be in /usr/share/licenses/common.
# Source: https://gitlab.archlinux.org/archlinux/packaging/packages/licenses/-
#         /commit/3c208d6eb23e2ad9590fc168f43a0d06ab47c6d6
LEGACY_COMMON_LICENSES = {
    "AGPL3": "AGPL-3.0-only",
    "APACHE": "Apache-2.0",
    "Apache": "Apache-2.0",
    "CCPL:by-sa": "CC-BY-SA-3.0",
    "FDL": "GFDL-1.2-only",
    "FDL1.3": "GFDL-1.3-only",
    "GFDL-1.3": "GFDL-1.3-only",
    "GPL": "GPL-2.0-only",
    "GPL-2.0": "GPL-2.0-only",
    "GPL2": "GPL-2.0-only",
    "GPL3": "GPL-3.0-only",
    "LGPL": "LGPL-2.1-only",
    "LGPL2.1": "LGPL-2.1-only",
    "LGPL3": "LGPL-3.0-only",
    "MPL": "MPL-1.1",
    "PerlArtistic": "Artistic-1.0-Perl",
    "PSF": "PSF-2.0",
}


class MissingLicense(Exception):
    """Raised when a common license cannot be found or a package has no license."""


def read_encoded_file(filename):
    """Read an encoded file, attempting utf-8 then latin1 encodings."""
    with open(filename, "rb") as f:
        contents = f.read()
        try:
            contents = contents.decode("utf-8")
        except UnicodeDecodeError:
            contents = contents.decode("latin1")
    return contents


def parse_packages(packages_file):
    """Parse a list of packages as generated by pacman -Qi."""
    packages = []
    entry = {}
    with open(packages_file, encoding="utf-8") as f:
        for l in f:
            l = l.strip()

            # Empty line separates entries.
            if len(l) == 0:
                if entry:
                    packages.append(entry)
                    entry = {}
                continue

            # Parse lines of the format:
            # URL             : https://fmt.dev
            m = re.match(r"^([^:]+) ?: (.*)$", l)
            if m:
                key = m.group(1).strip()
                value = m.group(2).strip()
                entry[key] = value
        if entry:
            packages.append(entry)

    return packages


def get_licenses(license_dir, package_name):
    """Get custom licenses for a given package from a directory of licenses."""
    licenses = []
    # Debug symbols use the same license as the parent package.
    if package_name.endswith("-debug"):
        package_name = package_name[:-6]
    package_dir = os.path.join(license_dir, package_name)
    if os.path.isdir(package_dir):
        # Directories may contain multiple licenses.  Get them all.
        for de in os.scandir(package_dir):
            filename = os.path.join(package_dir, de.name)
            contents = read_encoded_file(filename)
            licenses.append(
                {
                    "name": de.name,
                    "filename": filename,
                    "contents": contents,
                }
            )
    return licenses


def make_license_link(license_id):
    """Create an HTML link for a stock license."""
    return (
        f"<li><a href='#{license_id}'>Arch Stock License {license_id}</a></li>"
    )


def find_common_license(license_name, license_dir):
    """Find the path of a common license from a directory of Arch licenses."""

    # Map legacy common licenses to SPDX.
    license_name = LEGACY_COMMON_LICENSES.get(license_name, license_name)

    # Look for an SPDX license.
    file = os.path.join(license_dir, "spdx", f"{license_name}.txt")
    if os.path.isfile(file):
        return file

    # License not found.
    return None


def split_licenses(license_str: str) -> List[str]:
    """Split a license string from pacman -Qi"""
    # pacman -Qi prints licenses separated by two spaces.
    licenses = set()
    for chunk in re.split(r"(?:  | AND | OR | WITH )", license_str):
        # Strip parentheses.
        chunk = re.sub(r"[()]", "", chunk)
        # What's left is probably a license.
        licenses.add(chunk)
    return sorted(list(licenses))


def add_package_info(packages, license_dir):
    """Add licenses and additional data for each package."""
    license_error = False
    for p in packages:
        package_name = p["Name"]
        p["comments"] = ""
        p["name_version"] = f"{p['Name']} {p['Version']}"

        # Include any custom license text.
        license_text = ""
        custom_licenses = get_licenses(license_dir, package_name)
        for l in custom_licenses:
            license_text += l["contents"]
        p["license_text"] = license_text

        # Collect links to common licenses used by the package, and ensure that every package has a
        # valid license, either:
        # - special common licenses that requires a per-package license (eg BSD, MIT)
        # - a custom license that requires a per-package license
        common = set()
        ptr = ""
        for l in split_licenses(p["Licenses"]):
            if l in ("custom:none", "LicenseRef-None"):
                # Package has no license (which is invalid for anything other than the 'licenses'
                # package, and checked for below).
                continue
            if l in ("LicenseRef-PublicDomain", "LicenseRef-Sqlite"):
                # Public domain code has no license.
                continue
            if l in IGNORABLE_LICENSE_ALLOWLIST.get(package_name, []):
                # We can ignore this license.
                continue
            if (
                l.startswith("LicenseRef-")
                or l
                in (
                    "BSD",
                    "BSD-2-Clause",
                    "BSD-3-Clause",
                    "BSD-4-Clause-UC",
                    "ISC",
                    "MIT",
                    "ZLIB",
                    "Python",
                    "OFL",
                    "Zlib",
                )
                or l in CUSTOM_LICENSE_ALLOWLIST.get(package_name, [])
            ):
                # These licenses are special cases that are treated as custom licenses.
                # See: https://wiki.archlinux.org/title/PKGBUILD#license
                l = "custom:" + l
            if l == "custom" or l.startswith("custom:"):
                # Package has specified a custom license; make sure it provided one.
                assert custom_licenses, (
                    f"Package {package_name} specifies a custom license {l} but no license file "
                    f"was found in /usr/share/licenses/{package_name}"
                )
                continue
            # Package has specified a common license; add a link.
            if find_common_license(l, license_dir):
                common.add(l)
                ptr += make_license_link(l)
            else:
                license_error = True
                print(
                    f"Could not find license {l} for {package_name} in"
                    " /usr/share/licenses/spdx"
                )
                package_license_dir = os.path.join(license_dir, package_name)
                if not os.path.exists(package_license_dir):
                    print(
                        "There is no custom license directory for this package; this either\n"
                        "needs to go into the LEGACY_COMMON_LICENSES mapping in\n"
                        "generate_license_html.py, or is an upstream bug."
                    )
                    continue

                license_files = os.listdir(package_license_dir)
                if not license_files:
                    print(
                        "There is a custom license directory for this package, but it's empty;\n"
                        "this is probably an upstream bug."
                    )
                    continue

                print(
                    "If "
                    + (
                        "this file"
                        if len(license_files) == 1
                        else "one of these files"
                    )
                    + " looks like the correct license:"
                )
                for license_file in license_files:
                    print(f"    {license_file}:")
                    with open(
                        f"{package_license_dir}/{license_file}",
                        encoding="utf-8",
                    ) as f:
                        print(
                            "".join(
                                "        " + line
                                for line in f.readlines(1024)[:10]
                            )
                        )
                print(
                    "You might be able to fix this by adding this to the CUSTOM_LICENSE_ALLOWLIST\n"
                    "in generate_license_html.py:\n\n"
                    f"    # {l} license provided in /usr/share/licenses/{package_name}\n"
                    f'    "{package_name}": ["{l}"],\n'
                )

        # licenses package has custom:none and has just the common licenses.
        exceptions = {"licenses"}

        # Ensure that the package has a valid licenses if it is not an exception.
        if package_name not in exceptions and not (license_text or ptr):
            print(f"Package {package_name} has no license")
            license_error = True
            continue

        # Return processed license data for later use:
        # - A set of common license IDs, used later to generate the common license block.
        p["license_common"] = common
        # - An HTML string containing links to common licenses used by this package.
        p["license_ptr"] = ptr

    if license_error:
        raise MissingLicense("One or more licenses are missing")


def get_common_licenses(packages, license_dir):
    """Find all the common licenses referenced by a set of a packages."""
    # Find the set of common licenses keeping track of all the referencing
    # packages.
    common = {}
    for p in packages:
        for l in p["license_common"]:
            common.setdefault(l, []).append(p["name_version"])

    # Read the license text for each license.
    common_licenses = []
    for c in sorted(common.keys()):
        contents = read_encoded_file(find_common_license(c, license_dir))
        common_licenses.append(
            {
                "license_name": c,
                "license_type": "Arch Stock",
                "license": contents,
                "license_packages": " ".join(common[c]),
            }
        )
    return common_licenses


def generate_html(packages, common_licenses):
    """Generate an HTML credits for a list of packages and common licenses."""

    # Read all the templates.
    credits_template = string.Template(read_encoded_file(CREDITS_TEMPLATE))
    entry_template = string.Template(read_encoded_file(ENTRY_TEMPLATE))
    shared_template = string.Template(read_encoded_file(SHARED_TEMPLATE))

    # Create entries for each package.
    entries = ""
    for p in packages:
        entries += entry_template.substitute(p)

    # Create entries for each common license.
    common = ""
    for c in common_licenses:
        common += shared_template.substitute(c)

    # Create the final HTML page.
    env = {
        "tainted_warning_if_any": "",
        "entries": entries,
        "licenses": common,
    }
    credits_html = credits_template.substitute(env)
    return credits_html


def prepare_html(packages_txt, licenses_tar):
    """Prepare and generate HTML summary of licenses."""
    # Get packages and corresponding licenses.
    packages = parse_packages(packages_txt)
    add_package_info(packages, licenses_tar)
    common_licenses = get_common_licenses(packages, licenses_tar)

    # Generate the HTML and write to a given file.
    return generate_html(packages, common_licenses)


def main():
    """Command-line for generating a license summary."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "packages",
        default="packages.txt",
        help="Path to the list of packages (pacman -Qi)",
    )
    parser.add_argument(
        "licenses",
        default="licenses",
        help="Path to the licenses dir (/usr/share/licenses)",
    )
    parser.add_argument("--output", "-o", help="Path to write file to")

    args = parser.parse_args()

    html = prepare_html(args.packages, args.licenses)
    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(html)
    else:
        print(html)


if __name__ == "__main__":
    main()
