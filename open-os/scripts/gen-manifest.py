#!/usr/bin/env python3
"""Generate Manifest files for OPENOS ebuilds."""
import hashlib
import os
from pathlib import Path

OVERLAY = Path("/Users/cangcang/code/OpenOS/open-os/src/third_party/openos-overlay")

def hash_file(path, algo):
    h = hashlib.new(algo)
    with open(path, 'rb') as f:
        h.update(f.read())
    return h.hexdigest()

for ebuild_path in sorted(OVERLAY.glob("**/*.ebuild")):
    pkg_dir = ebuild_path.parent
    mf_path = pkg_dir / "Manifest"

    # Skip if manifest already exists
    if mf_path.exists():
        continue

    # Collect all files in the package dir (excluding Manifest itself)
    entries = []
    files_dir = pkg_dir / "files"

    # The ebuild file
    entries.append(f"EBUILD {ebuild_path.name} 0 BLAKE2B {hash_file(ebuild_path, 'blake2b')} SHA512 {hash_file(ebuild_path, 'sha512')}")

    # Files in files/ directory
    if files_dir.is_dir():
        for f in sorted(files_dir.iterdir()):
            if f.is_file():
                size = f.stat().st_size
                entries.append(f"AUX {f.name} {size} BLAKE2B {hash_file(f, 'blake2b')} SHA512 {hash_file(f, 'sha512')}")

    # Additional files in pkg_dir (like icons in openos-icons before moving)
    for f in sorted(pkg_dir.iterdir()):
        if f.is_file() and f.suffix in ('.svg', '.png', '.jpg', '.ttf', '.conf') and f.name != 'Manifest':
            size = f.stat().st_size
            entries.append(f"AUX {f.name} {size} BLAKE2B {hash_file(f, 'blake2b')} SHA512 {hash_file(f, 'sha512')}")

    mf_path.write_text("\n".join(entries) + "\n")
    print(f"Manifest: {mf_path.relative_to(OVERLAY)}")
