#!/usr/bin/env python3
"""OPENOS ebuild validator — syntax, structure, dependency checks."""
import re, os, sys
from pathlib import Path

OVERLAY = Path("/Users/cangcang/code/OpenOS/open-os/src/third_party/openos-overlay")

REQUIRED_VARS = ["EAPI", "DESCRIPTION", "LICENSE", "SLOT", "KEYWORDS"]

def validate_ebuild(path: Path) -> list[str]:
    issues = []
    content = path.read_text()
    lines = content.split("\n")

    # 1. Required variables
    for var in REQUIRED_VARS:
        if not re.search(rf"^{var}=", content, re.MULTILINE):
            issues.append(f"MISSING {var}")

    # 2. EAPI value
    m = re.search(r"^EAPI=(\S+)", content, re.MULTILINE)
    eapi = m.group(1).strip('"') if m else "?"
    if eapi not in ("7", "8"):
        issues.append(f"EAPI={eapi} (expected 7 or 8)")

    # 3. src_install() present (except meta packages)
    if "RDEPEND=" in content and "src_install()" not in content:
        # meta packages with only RDEPEND are fine without src_install
        if "src_compile()" not in content and "src_unpack()" not in content:
            pass  # meta package, OK
    else:
        if "src_install()" not in content:
            issues.append("MISSING src_install()")

    # 4. FILESDIR references resolution
    files_dir = path.parent / "files"
    for m in re.finditer(r'\$\{FILESDIR\}/(\S+?)(?:"|\s|\))', content):
        ref = m.group(1).strip('"').rstrip('"')
        if not (files_dir / ref).exists():
            issues.append(f"FILESDIR/{ref} NOT FOUND")

    # 5. Check for common ebuild mistakes
    if "\t" in content:
        issues.append("USES TABS (use spaces)")

    fn = path.name
    if not re.match(r"^[\w-]+-\d[\w.]*\.ebuild$", fn):
        issues.append(f"BAD FILENAME: {fn}")

    return issues

def validate_overlay():
    total, passed, failed = 0, 0, 0
    for cat in sorted(OVERLAY.glob("*/*/")):
        if not cat.is_dir():
            continue
        ebuilds = list(cat.glob("*.ebuild"))
        if not ebuilds:
            continue
        for eb in sorted(ebuilds):
            total += 1
            issues = validate_ebuild(eb)
            pkg = f"{cat.parent.name}/{cat.name}"
            if issues:
                failed += 1
                print(f"  FAIL  {pkg}/{eb.name}")
                for i in issues:
                    print(f"         → {i}")
            else:
                passed += 1
                print(f"  OK    {pkg}/{eb.name}")

    print(f"\n{'='*50}")
    print(f"Total: {total}  Passed: {passed}  Failed: {failed}")
    return failed == 0

def check_profile():
    """Check the overlay profiles/ structure."""
    profile = OVERLAY / "profiles"
    issues = []
    if not (profile / "base" / "make.defaults").exists():
        issues.append("profiles/base/make.defaults MISSING")
    if not (profile / "repo_name").exists():
        issues.append("profiles/repo_name MISSING")
    # layout.conf
    layout = OVERLAY / "metadata" / "layout.conf"
    if layout.exists():
        content = layout.read_text()
        if "masters =" not in content:
            issues.append("layout.conf: masters= MISSING")
        if "repo-name =" not in content:
            issues.append("layout.conf: repo-name= MISSING")
    else:
        issues.append("metadata/layout.conf MISSING")

    for i in issues:
        print(f"  WARN  {i}")
    return len(issues)

if __name__ == "__main__":
    ok = validate_overlay()
    warn_count = check_profile()
    sys.exit(0 if ok else 1)
