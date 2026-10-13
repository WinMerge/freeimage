#!/usr/bin/env python3
"""Build exact-checkout Arch inputs and distribution-specific Debian versions."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import subprocess

def git(root, *args):
    return subprocess.check_output(["git", "-C", str(root), *args], text=True).strip()

def prepare(root, output, distro):
    header = (root / "Source/FreeImage.h").read_text()
    parts = [re.search(rf"^#define FREEIMAGE_{name}\s+(\d+)", header, re.M).group(1)
             for name in ("MAJOR_VERSION", "MINOR_VERSION", "RELEASE_SERIAL")]
    version = ".".join(parts)
    fallback = re.search(r"set\(FREEIMAGE_VERSION_FALLBACK ([0-9.]+)\)",
                         (root / "CMakeLists.txt").read_text()).group(1)
    if fallback != version:
        raise ValueError("CMake archive fallback must match Source/FreeImage.h")
    head = git(root, "rev-parse", "HEAD")
    epoch = git(root, "show", "-s", "--format=%ct", "HEAD")
    release = os.environ.get("GITHUB_REF_TYPE") == "tag"
    if release:
        tag = os.environ.get("GITHUB_REF_NAME", "")
        if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", tag) or tag != version:
            raise ValueError("Release tag must match Source/FreeImage.h exactly")
        if git(root, "rev-parse", f"refs/tags/{tag}^{{commit}}") != head:
            raise ValueError("Checkout does not match release tag")
    package_version = version if release else f"{version}~git{head[:8]}"
    output.mkdir(parents=True, exist_ok=True)
    archive = output / f"FreeImage-{version}-linux-source.tar.gz"
    subprocess.run(["git", "-C", str(root), "archive", "--format=tar.gz",
                    f"--prefix=FreeImage-{version}/", "-o", str(archive), "HEAD"], check=True)
    recipe = (root / "packaging/arch/PKGBUILD").read_text()
    recipe = re.sub(r"^pkgver=.*$", f"pkgver={version}", recipe, flags=re.M)
    recipe = re.sub(r"^pkgrel=.*$", "pkgrel=1" if release else f"pkgrel=0.{epoch}", recipe, flags=re.M)
    recipe = re.sub(r"^source=\(.*?\)\nsha256sums=\(.*?\)", 
                    f'source=("FreeImage-$pkgver-linux-source.tar.gz")\n'
                    f"sha256sums=('{hashlib.sha256(archive.read_bytes()).hexdigest()}')",
                    recipe, flags=re.M | re.S)
    recipe = re.sub(r"\nprepare\(\) \{.*?\n\}\n", "\n", recipe, flags=re.S)
    (output / "PKGBUILD").write_text(recipe)
    if distro:
        if not re.fullmatch(r"[a-z0-9.]+", distro):
            raise ValueError("Invalid distribution suffix")
        changelog = root / "debian/changelog"
        data = changelog.read_text()
        data = re.sub(r"^(freeimage-maintained) \([^)]*\)",
                      rf"\1 ({package_version}-1~{distro})", data, count=1)
        changelog.write_text(data)
    print(f"version={version}, release={release}, commit={head}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--distro")
    args = parser.parse_args()
    prepare(args.root.resolve(), args.output.resolve(), args.distro)
