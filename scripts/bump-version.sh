#!/usr/bin/env bash
# Keep release metadata in sync; Python's byte I/O preserves CRLF and cp1252.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
command -v python3 >/dev/null 2>&1 || { echo "python3 is required" >&2; exit 1; }

python3 - "$REPO_ROOT" "$@" <<'PY'
import argparse
import difflib
from pathlib import Path
import re
import sys

root = Path(sys.argv[1])
parser = argparse.ArgumentParser(
    description="Bump FreeImage release metadata (default: next patch). "
                "Requires Python 3. Does not commit, tag, or publish.")
parser.add_argument("version", nargs="?", help="MAJOR.MINOR.PATCH, without a v prefix")
parser.add_argument("--dry-run", action="store_true", help="show the diff without writing files")
args = parser.parse_args(sys.argv[2:])


def replace_once(data, pattern, replacement, label):
    result, count = re.subn(pattern, replacement, data, flags=re.MULTILINE)
    if count != 1:
        raise ValueError(f"{label}: expected one matching version field, found {count}")
    return result


def prepare():
    header_path = root / "Source/FreeImage.h"
    header = header_path.read_bytes()
    names = ("MAJOR_VERSION", "MINOR_VERSION", "RELEASE_SERIAL")
    fields = [re.findall(rb"^#define FREEIMAGE_" + name.encode() + rb"[ \t]+([0-9]+)[ \t]*\r?$",
                         header, re.MULTILINE) for name in names]
    if any(len(field) != 1 for field in fields):
        raise ValueError("Source/FreeImage.h: missing or duplicated version definitions")
    current = tuple(int(field[0]) for field in fields)
    if args.version is None:
        target = (*current[:2], current[2] + 1)
    else:
        if not re.fullmatch(r"(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)", args.version):
            raise ValueError("version must be MAJOR.MINOR.PATCH without leading zeros or a suffix")
        target = tuple(map(int, args.version.split(".")))
    if target <= current:
        raise ValueError("new version must be greater than the current version")
    if any(part > 65535 for part in target):
        raise ValueError("version components must fit Windows resource fields (0..65535)")
    old = ".".join(map(str, current)).encode()
    new = ".".join(map(str, target)).encode()
    if len(new) >= 16:
        raise ValueError("version must fit FreeImage_GetVersion()'s 16-byte buffer")
    changes = {}

    for name, value in zip(names, target):
        header = replace_once(header, rb"^(#define FREEIMAGE_" + name.encode() + rb"[ \t]+)[0-9]+",
                              lambda m: m[1] + str(value).encode(), "Source/FreeImage.h")
    changes["Source/FreeImage.h"] = header

    cmake = (root / "CMakeLists.txt").read_bytes()
    cmake = replace_once(cmake, rb"^set\(FREEIMAGE_VERSION_FALLBACK " + re.escape(old) + rb"\)",
                         b"set(FREEIMAGE_VERSION_FALLBACK " + new + b")", "CMakeLists.txt")
    # Only refresh the comments beside the fallback, not unrelated describe examples.
    cmake = replace_once(cmake, rb'^(# Determine the version from the latest git tag \(e\.g\. ")[0-9.]+',
                         lambda m: m[1] + new, "CMakeLists.txt tag example")
    cmake = replace_once(cmake, rb'^( +# e\.g\. ")[0-9.]+(-45-ge32af9e" -> tag )[0-9.]+',
                         lambda m: m[1] + new + m[2] + new, "CMakeLists.txt describe example")
    changes["CMakeLists.txt"] = cmake

    old_numbers = b",".join(str(part).encode() for part in (*current, 0))
    new_numbers = b",".join(str(part).encode() for part in (*target, 0))
    for path in ("FreeImage.rc", "Wrapper/FreeImagePlus/FreeImagePlus.rc"):
        data = (root / path).read_bytes()
        for field in (b"FILEVERSION", b"PRODUCTVERSION"):
            data = replace_once(data, rb"^([ \t]*" + field + rb"[ \t]+)" + re.escape(old_numbers) + rb"(?=[ \t]*\r?$)",
                                lambda m: m[1] + new_numbers, path)
        for field in (b"FileVersion", b"ProductVersion"):
            data = replace_once(data, rb'(VALUE "' + field + rb'", ")' + re.escape(old_numbers.replace(b",", b", ")) + rb'(?=\\0")',
                                lambda m: m[1] + new_numbers.replace(b",", b", "), path)
        changes[path] = data

    readme = (root / "README.md").read_bytes()
    for prefix, suffix in ((b"This tree is FreeImage **", b"**"),
                           (b"falls back to ", b" when the archive")):
        readme = replace_once(readme, re.escape(prefix + old + suffix),
                              prefix + new + suffix, "README.md")
    changes["README.md"] = readme
    return old.decode(), new.decode(), changes


try:
    old, new, changes = prepare()  # Validate every file before writing any of them.
    print(f"FreeImage {old} -> {new}" + (" (dry run)" if args.dry_run else ""))
    for path, updated in changes.items():
        original = (root / path).read_bytes()
        if args.dry_run:
            sys.stdout.writelines(difflib.unified_diff(
                original.decode("latin1").splitlines(keepends=True),
                updated.decode("latin1").splitlines(keepends=True),
                fromfile="a/" + path, tofile="b/" + path))
        else:
            (root / path).write_bytes(updated)
            print(f"Updated {path}")
    print("Next: recheck vendored versions and update THIRD_PARTY_VERSIONS.md; "
          "see AGENTS.md for the release checklist.")
    print("CMake in a git checkout describes the previous tag until the release tag exists.")
except (OSError, ValueError) as error:
    parser.exit(1, f"error: {error}\n")
PY
