import importlib.util
import os
from pathlib import Path
import re
import subprocess
import tarfile
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("release_inputs", ROOT / "packaging/scripts/prepare-ci.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class ReleaseInputs(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / "repo"
        self.root.mkdir()
        (self.root / "Source").mkdir()
        (self.root / "packaging/arch").mkdir(parents=True)
        (self.root / "debian").mkdir()
        (self.root / "Source/FreeImage.h").write_text(
            "#define FREEIMAGE_MAJOR_VERSION 3\n"
            "#define FREEIMAGE_MINOR_VERSION 19\n"
            "#define FREEIMAGE_RELEASE_SERIAL 21\n")
        (self.root / "CMakeLists.txt").write_text("set(FREEIMAGE_VERSION_FALLBACK 3.19.21)\n")
        (self.root / "packaging/arch/PKGBUILD").write_bytes((ROOT / "packaging/arch/PKGBUILD").read_bytes())
        (self.root / "debian/changelog").write_text("freeimage-maintained (old) unstable; urgency=medium\n")
        self.git("init", "-q")
        self.git("add", ".")
        self.git("-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
                 "commit", "-qm", "fixture")
        self.output = Path(self.temp.name) / "output"

    def git(self, *args):
        return subprocess.check_output(["git", "-C", str(self.root), *args], text=True).strip()

    def run_prepare(self, ref_type="branch", ref_name="", distro="ubuntu24.04"):
        with patch.dict(os.environ, {"GITHUB_REF_TYPE": ref_type, "GITHUB_REF_NAME": ref_name}):
            module.prepare(self.root, self.output, distro)

    def test_preview_uses_exact_checkout_and_no_backport(self):
        self.run_prepare()
        recipe = (self.output / "PKGBUILD").read_text()
        self.assertIn("pkgrel=0.", recipe)
        self.assertNotIn("linux-install.patch", recipe)
        self.assertNotIn("prepare()", recipe)
        self.assertIn("3.19.21~git", (self.root / "debian/changelog").read_text())
        with tarfile.open(self.output / "FreeImage-3.19.21-linux-source.tar.gz") as archive:
            header = archive.extractfile("FreeImage-3.19.21/Source/FreeImage.h").read()
        self.assertEqual(header, (self.root / "Source/FreeImage.h").read_bytes())

    def test_valid_release_has_distinct_distribution_versions(self):
        self.git("tag", "3.19.21")
        self.run_prepare("tag", "3.19.21", "ubuntu24.04")
        ubuntu = (self.root / "debian/changelog").read_text()
        self.run_prepare("tag", "3.19.21", "debian13")
        debian = (self.root / "debian/changelog").read_text()
        self.assertIn("3.19.21-1~ubuntu24.04", ubuntu)
        self.assertIn("3.19.21-1~debian13", debian)
        self.assertIn("pkgrel=1\n", (self.output / "PKGBUILD").read_text())

    def test_wrong_version_or_non_numeric_tag_is_rejected(self):
        self.git("tag", "3.19.22")
        for tag in ("3.19.22", "v3.19.21", "3.19.21-rc1"):
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                self.run_prepare("tag", tag)

    def test_tag_must_identify_checkout(self):
        self.git("tag", "3.19.21")
        (self.root / "extra").write_text("new commit")
        self.git("add", "extra")
        self.git("-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
                 "commit", "-qm", "next")
        with self.assertRaises(ValueError):
            self.run_prepare("tag", "3.19.21")

    def test_archive_fallback_must_match_header(self):
        (self.root / "CMakeLists.txt").write_text("set(FREEIMAGE_VERSION_FALLBACK 3.19.20)\n")
        with self.assertRaises(ValueError):
            self.run_prepare()

if __name__ == "__main__":
    unittest.main()
