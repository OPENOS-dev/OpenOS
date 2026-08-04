#!/usr/bin/env python3
"""OPENOS local install simulator — mirrors each ebuild src_install() into a
staging root so we can verify the build produces correct outputs."""
import os, shutil, sys
from pathlib import Path

OVERLAY = Path("/Users/cangcang/code/OpenOS/open-os/src/third_party/openos-overlay")
STAGING  = Path("/tmp/openos-buildroot")

def clean():
    if STAGING.exists():
        shutil.rmtree(STAGING)
    STAGING.mkdir(parents=True)

def insinto(dest):
    """Mimics Portage 'insinto' — sets install destination."""
    return STAGING / dest.lstrip("/")

def doins(dest, *srcs):
    """Mimics Portage 'doins' — copies files into destination."""
    os.makedirs(dest, exist_ok=True)
    for pattern in srcs:
        # Resolve by treating as glob
        import glob as g
        parent = pattern.parent if isinstance(pattern, Path) else Path(pattern).parent
        matches = list(g.glob(str(pattern)))
        for m in matches:
            src = Path(m)
            if src.is_file():
                shutil.copy2(src, dest / src.name)

def exeinto(dest):
    """Mimics Portage 'exeinto' — sets install destination for executables."""
    return STAGING / dest.lstrip("/")

def doexe(dest, *srcs):
    """Mimics Portage 'doexe' — copies executables with perms."""
    os.makedirs(dest, exist_ok=True)
    for pattern in srcs:
        import glob as g
        for m in g.glob(str(pattern)):
            src = Path(m)
            if src.is_file():
                dst = dest / src.name
                shutil.copy2(src, dst)
                dst.chmod(0o755)

def dosym(target, link):
    """Mimics Portage 'dosym'."""
    link_path = STAGING / link.lstrip("/")
    os.makedirs(link_path.parent, exist_ok=True)
    if link_path.exists() or link_path.is_symlink():
        link_path.unlink()
    link_path.symlink_to(target)

# ────────────────────────────────────────────────
# openos-theme: CSS files + symlinks
# ────────────────────────────────────────────────
def install_theme():
    print("\n[openos-theme] Installing theme CSS ...")
    files = Path(OVERLAY, "chromeos-base/openos-theme/files")
    dest = insinto("/usr/share/openos/theme")
    for css in sorted(files.glob("*.css")):
        doins(dest, css)
        print(f"  {css.name} → {dest}")
    dosym("/usr/share/openos/theme/login.css",  "/usr/share/chromeos-assets/login/nothing-login.css")
    dosym("/usr/share/openos/theme/tokens.css", "/usr/share/chromeos-assets/login/nothing-tokens.css")
    dosym("/usr/share/openos/theme/shell.css",  "/usr/share/chromeos-assets/shell/nothing-shell.css")
    dosym("/usr/share/openos/theme/webui.css",  "/usr/share/chromeos-assets/webui/nothing-webui.css")
    print("  4 symlinks created")

# ────────────────────────────────────────────────
# openos-setup: init script + upstart conf
# ────────────────────────────────────────────────
def install_setup():
    print("\n[openos-setup] Installing first-boot scripts ...")
    files = Path(OVERLAY, "chromeos-base/openos-setup/files")
    dest = exeinto("/usr/libexec/openos")
    doexe(dest, files / "openos-first-boot.sh")
    print(f"  openos-first-boot.sh → {dest}")
    dest2 = insinto("/etc/init")
    doins(dest2, files / "openos-first-boot.conf")
    print(f"  openos-first-boot.conf → {dest2}")

# ────────────────────────────────────────────────
# openos-icons: SVG logos
# ────────────────────────────────────────────────
def install_icons():
    print("\n[openos-icons] Installing brand icons ...")
    files = Path(OVERLAY, "x11-themes/openos-icons/files")
    dest = insinto("/usr/share/openos/icons")
    for svg in sorted(files.glob("*.svg")):
        doins(dest, svg)
        print(f"  {svg.name} → {dest}")

# ────────────────────────────────────────────────
# openos-meta: nothing to install (dependency only)
# ────────────────────────────────────────────────
def install_meta():
    print("\n[openos-meta] Meta package — dependency binding (no files)")

# ────────────────────────────────────────────────
def show_tree(root=None, prefix=""):
    if root is None:
        root = STAGING
    entries = sorted(root.iterdir())
    for i, entry in enumerate(entries):
        connector = "└── " if i == len(entries) - 1 else "├── "
        if entry.is_symlink():
            print(f"{prefix}{connector}{entry.name} → {os.readlink(entry)}")
        elif entry.is_dir():
            print(f"{prefix}{connector}{entry.name}/")
            show_tree(entry, prefix + ("    " if i == len(entries) - 1 else "│   "))
        else:
            size = entry.stat().st_size
            print(f"{prefix}{connector}{entry.name}  ({size} B)")

if __name__ == "__main__":
    clean()
    print("=== OPENOS Local Install Simulator ===")
    print(f"Staging root: {STAGING}")

    install_theme()
    install_setup()
    install_icons()
    install_meta()

    total = sum(1 for _ in STAGING.rglob("*") if _.is_file() or _.is_symlink())
    print(f"\n=== Complete: {total} files installed ===\n")
    print("STAGING TREE:")
    show_tree()
