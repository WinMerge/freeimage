#!/usr/bin/env python3
"""Exercise release edits in disposable trees, including byte preservation."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
FILES = ("Source/FreeImage.h", "CMakeLists.txt", "FreeImage.rc",
         "Wrapper/FreeImagePlus/FreeImagePlus.rc", "README.md")


class BumpVersionTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="freeimage release ")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name in (*FILES, "scripts/bump-version.sh"):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, path)
        # Include a legacy cp1252 byte and verify it survives with the CRLFs.
        with (self.root / "FreeImage.rc").open("ab") as stream:
            stream.write(b"// Copyright \xa9\r\n")
        self.before = self.snapshot()
        header = self.before["Source/FreeImage.h"]
        self.version = tuple(int(re.search(rb"#define FREEIMAGE_" + name + rb"\s+(\d+)", header)[1])
                             for name in (b"MAJOR_VERSION", b"MINOR_VERSION", b"RELEASE_SERIAL"))

    def snapshot(self):
        return {name: (self.root / name).read_bytes() for name in FILES}

    def run_bump(self, *args, success=True):
        result = subprocess.run(["bash", str(self.root / "scripts/bump-version.sh"), *args],
                                cwd="/", capture_output=True, text=True)
        self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
        return result

    def test_dry_run(self):
        result = self.run_bump("--dry-run")
        self.assertIn("(dry run)", result.stdout)
        self.assertIn("--- a/FreeImage.rc", result.stdout)
        self.assertEqual(self.snapshot(), self.before)

    def test_next_patch_preserves_other_bytes(self):
        self.run_bump()
        after = self.snapshot()
        for name in FILES:
            # Only decimal digits may change; CRLF, cp1252 and all other bytes survive.
            self.assertEqual(re.sub(rb"\d+", b"#", self.before[name]),
                             re.sub(rb"\d+", b"#", after[name]), name)
        patch = str(self.version[2] + 1).encode()
        self.assertRegex(after["Source/FreeImage.h"], rb"FREEIMAGE_RELEASE_SERIAL\s+" + patch + rb"\b")

    def test_explicit_major_and_minor(self):
        target = f"{self.version[0] + 1}.2.0"
        self.run_bump(target)
        self.assertIn(f"FREEIMAGE_VERSION_FALLBACK {target}".encode(), self.snapshot()["CMakeLists.txt"])
        self.assertIn(f"FILEVERSION {self.version[0] + 1},2,0,0".encode(), self.snapshot()["FreeImage.rc"])

    def test_invalid_versions_write_nothing(self):
        for version in ("v3.19.30", "3.019.30", "3.19", "3.19.30-rc1", "0.0.0",
                        ".".join(map(str, self.version)), "65536.0.0", "65535.65535.65535"):
            with self.subTest(version=version):
                self.run_bump(version, success=False)
                self.assertEqual(self.snapshot(), self.before)

    def test_inconsistent_metadata_writes_nothing(self):
        path = self.root / "README.md"  # The last file validated.
        path.write_bytes(path.read_bytes().replace(b"falls back to ", b"fallback "))
        before = self.snapshot()
        self.run_bump(success=False)
        self.assertEqual(self.snapshot(), before)


if __name__ == "__main__":
    unittest.main()
