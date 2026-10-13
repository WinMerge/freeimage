#!/usr/bin/env python3
"""Refresh the release backport and its checksum after CMake changes."""
from pathlib import Path
import difflib
import hashlib
import re
import subprocess

root = Path(__file__).resolve().parents[2]
recipe = root / "packaging/arch/PKGBUILD"
data = recipe.read_text()
version = re.search(r"^pkgver=(.+)$", data, re.M).group(1)
patch = []
for name in ("CMakeLists.txt", "cmake/freeimage.pc.in"):
    result = subprocess.run(["git", "show", f"{version}:{name}"], cwd=root, capture_output=True)
    old = result.stdout.decode() if result.returncode == 0 else ""
    new = (root / name).read_text()
    patch.extend(difflib.unified_diff(old.splitlines(True), new.splitlines(True),
                 fromfile=f"a/{name}" if old else "/dev/null", tofile=f"b/{name}"))
target = root / "packaging/arch/linux-install.patch"
target.write_text("".join(patch))
digest = hashlib.sha256(target.read_bytes()).hexdigest()
data = re.sub(r"(sha256sums=\('[0-9a-f]{64}' ')[0-9a-f]{64}('\))",
              lambda m: m[1] + digest + m[2], data)
recipe.write_text(data)
