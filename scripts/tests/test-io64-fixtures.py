#!/usr/bin/env python3
"""Per-case packaging, sparse extraction and cleanup tests; no FreeImage build."""

import hashlib
import http.client
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest import mock
import urllib.error

spec = importlib.util.spec_from_file_location("fixtures", Path(__file__).with_name("io64-fixtures.py"))
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)


class FixtureTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.assets = self.root / "assets"
        self.records = fixtures.build(self.assets)

    def unpack(self, record, destination=None):
        destination = destination or self.root / "unpacked"
        return fixtures.extract(self.assets / record["asset"]["name"], destination, record)

    def test_deterministic_independent_assets(self):
        second = self.root / "second"
        fixtures.build(second)
        for asset in self.assets.iterdir():
            self.assertEqual(asset.read_bytes(), (second / asset.name).read_bytes())
        names = [r["asset"]["name"] for r in self.records]
        self.assertEqual(len(names), len(set(names)))
        self.assertLess(sum(r["asset"]["size"] for r in self.records), 128 * 1024)
        self.assertEqual(len(list(self.assets.glob("*.tar.gz"))), len(self.records))
        for record in self.records:
            with tarfile.open(self.assets / record["asset"]["name"]) as tar:
                self.assertEqual(tar.getnames(), [record["path"]])

    def test_checksums(self):
        for line in (self.assets / "SHA256SUMS").read_text().splitlines():
            digest, name = line.split(maxsplit=1)
            self.assertEqual(digest, hashlib.sha256((self.assets / name).read_bytes()).hexdigest())
        manifest = self.assets / "manifest.json"
        self.assertEqual(fixtures.load_manifest(manifest, hashlib.sha256(manifest.read_bytes()).hexdigest()), self.records)
        with self.assertRaisesRegex(ValueError, "Manifest SHA-256"):
            fixtures.load_manifest(manifest, "0" * 64)

    def test_round_trip_sparse_payloads_one_at_a_time(self):
        paths = [record["path"] for record in self.records]
        self.assertEqual(len(paths), len(set(paths)))
        self.assertGreaterEqual(len(paths), 80)
        self.assertEqual(sum(path.startswith("paths/") for path in paths), 60)
        self.assertIn("paths/emoji-\U0001f9ea/valid image.tif", paths)
        for record in self.records:
            with tempfile.TemporaryDirectory(dir=self.root) as case:
                path = self.unpack(record, Path(case) / "input")
                self.assertEqual(path.stat().st_size, record["logical_size"])
                if os.name != "nt":
                    self.assertLess(path.stat().st_blocks * 512, 1024 * 1024)
                elif record["logical_size"] > 2**32:
                    import ctypes
                    from ctypes import wintypes
                    attributes = ctypes.windll.kernel32.GetFileAttributesW
                    attributes.argtypes = [wintypes.LPCWSTR]
                    attributes.restype = wintypes.DWORD
                    self.assertTrue(attributes(str(path)) & 0x200, "Windows file must be sparse")
            self.assertFalse(path.exists())

    def test_sparse_holes_are_zero_and_last_byte_survives(self):
        record = next(r for r in self.records if r["path"] == "valid/bigtiff-strip-above-4g.tif")
        with self.unpack(record).open("rb") as file:
            file.seek(0x80000000)
            self.assertEqual(file.read(32), bytes(32))
            file.seek(-3, 2)
            self.assertEqual(file.read(), bytes(fixtures.RGB))

    def test_boundary_and_malformed_coverage(self):
        large = [r for r in self.records if r["logical_size"] > 2**32]
        self.assertTrue(any(r["expect"] == "decode" for r in large))
        self.assertTrue(any(r["expect"] == "reject" for r in large))
        for fmt in ("TIFF", "PNG", "BMP"):
            self.assertEqual({r["expect"] for r in self.records if r["format"] == fmt}, {"decode", "reject"})
        self.assertTrue(any("exif_user_comment" in r for r in self.records))
        self.assertTrue(any("big-endian" in r["path"] for r in self.records))
        known = [r for r in self.records if "known_behavior" in r]
        self.assertEqual(len(known), 4)
        self.assertTrue(all(r["expect"] == "reject" for r in known))

    def test_corruption_rejected_before_creating_files(self):
        record = self.records[0]
        archive = self.assets / record["asset"]["name"]
        archive.write_bytes(archive.read_bytes() + b"corruption")
        with self.assertRaisesRegex(ValueError, "SHA-256"):
            self.unpack(record)
        self.assertFalse((self.root / "unpacked").exists())

    def test_existing_output_is_never_overwritten(self):
        self.unpack(self.records[0])
        with self.assertRaises(FileExistsError):
            self.unpack(self.records[0])

    def test_wrong_payload_rejected(self):
        record = dict(self.records[0], path="different.tif")
        with self.assertRaisesRegex(ValueError, "selected fixture"):
            self.unpack(record)

    def test_rejects_software_release_tags(self):
        for tag in ["3.19.29", "v3.19.29", "test-fixtures-io64-v0", "../v1", "v1\n"]:
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                fixtures.build(self.root / "invalid", tag)

    def test_rejects_unsafe_member_names(self):
        for name in ["../outside", "/absolute", "C:/outside", "dir\\outside", "a/../../outside"]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                fixtures.safe_path(self.root, name)

    def test_rejects_links_in_archive(self):
        record = dict(self.records[0])
        archive = self.root / record["asset"]["name"]
        with tarfile.open(archive, "w:gz") as tar:
            member = tarfile.TarInfo(record["path"])
            member.type = tarfile.SYMTYPE
            member.linkname = "../outside"
            tar.addfile(member)
        record["asset"] = dict(record["asset"], sha256=hashlib.sha256(archive.read_bytes()).hexdigest())
        with self.assertRaisesRegex(ValueError, "Unsupported archive member"):
            fixtures.extract(archive, self.root / "unsafe", record)

    def test_native_tar_compatibility_one_payload_at_a_time(self):
        if os.name == "nt":
            self.skipTest("Windows uses the extractor that explicitly marks files sparse")
        tar = shutil.which("tar")
        if not tar:
            self.skipTest("No system tar")
        for record in self.records:
            with tempfile.TemporaryDirectory(dir=self.root) as case:
                subprocess.run([tar, "-xzf", str(self.assets / record["asset"]["name"]), "-C", case], check=True)
                path = Path(case) / record["path"]
                self.assertEqual(path.stat().st_size, record["logical_size"])
                self.assertLess(path.stat().st_blocks * 512, 1024 * 1024)
                with path.open("rb") as file:
                    for extent in record["extents"]:
                        file.seek(extent["offset"])
                        self.assertEqual(hashlib.sha256(file.read(extent["length"])).hexdigest(), extent["sha256"])

    def test_runner_cleans_each_case_and_continues_after_failure(self):
        scratch = self.root / "scratch"
        scratch.mkdir()
        log = self.root / "cases.jsonl"
        command = self.root / "decoder.py"
        command.write_text(
            "import json, pathlib, sys\n"
            "path, root, log = map(pathlib.Path, sys.argv[1:4])\n"
            "assert path.is_file()\n"
            "assert len(list(root.iterdir())) == 1\n"
            "with log.open('a', encoding='utf-8') as out: out.write(json.dumps(str(path)) + '\\n')\n"
            "sys.exit(1 if sys.argv[4] == 'reject' else 0)\n", encoding="utf-8")
        selected = [next(r for r in self.records if r["path"] == path) for path in
                    ["valid/bigtiff-strip-above-4g.tif", "broken/empty.tif", "valid/small.png"]]
        failed = fixtures.run_cases(selected, [sys.executable, str(command), "{path}", str(scratch), str(log), "{expect}"],
                                    payload_dir=self.assets, temporary_root=scratch)
        self.assertEqual(failed, 1)
        self.assertEqual(list(scratch.iterdir()), [])
        paths = [Path(json.loads(line)) for line in log.read_text(encoding="utf-8").splitlines()]
        self.assertEqual(len(paths), 3)
        self.assertTrue(all(not path.exists() for path in paths))

    def test_runner_cleans_after_timeout(self):
        scratch = self.root / "scratch"
        scratch.mkdir()
        failed = fixtures.run_cases(self.records[:1], [sys.executable, "-c", "import time; time.sleep(10)"],
                                    payload_dir=self.assets, temporary_root=scratch, timeout=0.1)
        self.assertEqual(failed, 1)
        self.assertEqual(list(scratch.iterdir()), [])

    def test_runner_downloads_only_the_current_payload(self):
        from unittest import mock
        import io
        selected = self.records[:3]
        downloads = []
        previous = []
        scratch = self.root / "downloads"
        scratch.mkdir()

        def download(url, timeout):
            name = url.rsplit("/", 1)[1]
            self.assertEqual(timeout, 60)
            self.assertTrue(all(not path.exists() for path in previous))
            self.assertEqual(len(list(scratch.iterdir())), 1)
            previous.extend(scratch.iterdir())
            downloads.append(name)
            return io.BytesIO((self.assets / name).read_bytes())

        with mock.patch.object(fixtures.urllib.request, "urlopen", side_effect=download):
            failed = fixtures.run_cases(selected, [sys.executable, "-c", "pass"],
                                        release_url="https://example.test/tag", temporary_root=scratch)
        self.assertEqual(failed, 0)
        self.assertEqual(downloads, [r["asset"]["name"] for r in selected])
        self.assertEqual(list(scratch.iterdir()), [])

    def test_known_failures_require_opt_in_and_manifest_entry(self):
        import contextlib
        import io
        known = next(r for r in self.records if "known_behavior" in r)
        ordinary = next(r for r in self.records if "known_behavior" not in r)
        command = [sys.executable, "-c", "raise SystemExit(3)"]
        output = io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            self.assertEqual(fixtures.run_cases([known], command, payload_dir=self.assets), 1)
            self.assertEqual(fixtures.run_cases([known], command, payload_dir=self.assets,
                                                allow_known_failures=True), 0)
            self.assertEqual(fixtures.run_cases([ordinary], command, payload_dir=self.assets,
                                                allow_known_failures=True), 1)
        self.assertIn("XFAIL " + known["path"], output.getvalue())
        self.assertIn("0 passed; 1 known failures; 0 unexpected failures", output.getvalue())

    def test_download_recovers_from_transient_errors(self):
        record = self.records[0]
        payload = (self.assets / record["asset"]["name"]).read_bytes()
        for error in [urllib.error.HTTPError("https://example.test", code, "temporary", {}, None)
                      for code in (408, 429, 500, 502, 503, 504)] + [
                          urllib.error.URLError("connection reset"), TimeoutError("read timed out")]:
            with self.subTest(error=error), \
                    mock.patch.object(fixtures.urllib.request, "urlopen",
                                      side_effect=[error, io.BytesIO(payload)]) as request, \
                    mock.patch.object(fixtures.time, "sleep") as sleep:
                self.assertEqual(fixtures.download("https://example.test/fixture"), payload)
                self.assertEqual(request.call_count, 2)
                sleep.assert_called_once_with(1)

    def test_download_restarts_after_interrupted_read(self):
        for error in [ConnectionResetError("interrupted"), http.client.IncompleteRead(b"partial", 10)]:
            response = mock.MagicMock()
            response.__enter__.return_value.read.side_effect = error
            with self.subTest(error=error), \
                    mock.patch.object(fixtures.urllib.request, "urlopen",
                                      side_effect=[response, io.BytesIO(b"complete")]), \
                    mock.patch.object(fixtures.time, "sleep"):
                self.assertEqual(fixtures.download("https://example.test/fixture"), b"complete")
            response.__exit__.assert_called_once()

    def test_runner_decodes_after_http_500_and_cleans_up(self):
        record = self.records[0]
        payload = (self.assets / record["asset"]["name"]).read_bytes()
        scratch = self.root / "recovered-download"
        scratch.mkdir()
        error = urllib.error.HTTPError("https://example.test", 500, "temporary", {}, None)
        with mock.patch.object(fixtures.urllib.request, "urlopen",
                               side_effect=[error, io.BytesIO(payload)]) as request, \
                mock.patch.object(fixtures.time, "sleep"), \
                mock.patch.object(fixtures.subprocess, "run") as decoder:
            self.assertEqual(fixtures.run_cases([record], ["decoder", "{path}"],
                                                release_url="https://example.test/tag",
                                                temporary_root=scratch), 0)
        self.assertEqual(request.call_count, 2)
        decoder.assert_called_once()
        self.assertEqual(list(scratch.iterdir()), [])

    def test_download_does_not_retry_oversized_response(self):
        with mock.patch.object(fixtures.urllib.request, "urlopen",
                               return_value=io.BytesIO(bytes(1024**2 + 1))) as request, \
                mock.patch.object(fixtures.time, "sleep") as sleep:
            with self.assertRaisesRegex(ValueError, "exceeds 1 MiB"):
                fixtures.download("https://example.test/fixture")
        request.assert_called_once()
        sleep.assert_not_called()

    def test_runner_cleans_after_download_retries_exhausted(self):
        scratch = self.root / "failed-download"
        scratch.mkdir()
        error = urllib.error.HTTPError("https://example.test", 500, "temporary", {}, None)
        with mock.patch.object(fixtures.urllib.request, "urlopen", side_effect=error) as request, \
                mock.patch.object(fixtures.time, "sleep") as sleep, \
                mock.patch.object(fixtures.subprocess, "run") as decoder:
            self.assertEqual(fixtures.run_cases(self.records[:1], ["decoder"],
                                                release_url="https://example.test/tag",
                                                temporary_root=scratch), 1)
        self.assertEqual(request.call_count, 4)
        self.assertEqual(sleep.call_args_list, [mock.call(1), mock.call(2), mock.call(4)])
        decoder.assert_not_called()
        self.assertEqual(list(scratch.iterdir()), [])

    def test_download_does_not_retry_permanent_http_errors(self):
        for code in (401, 403, 404):
            error = urllib.error.HTTPError("https://example.test", code, "permanent", {}, None)
            with self.subTest(code=code), \
                    mock.patch.object(fixtures.urllib.request, "urlopen", side_effect=error) as request, \
                    mock.patch.object(fixtures.time, "sleep") as sleep:
                with self.assertRaises(urllib.error.HTTPError):
                    fixtures.download("https://example.test/fixture")
                request.assert_called_once()
                sleep.assert_not_called()

    def test_runner_does_not_retry_corrupt_downloads(self):
        with mock.patch.object(fixtures.urllib.request, "urlopen",
                               return_value=io.BytesIO(b"corrupt archive")) as request, \
                mock.patch.object(fixtures.subprocess, "run") as decoder:
            self.assertEqual(fixtures.run_cases(self.records[:1], ["decoder"],
                                                release_url="https://example.test/tag"), 1)
        request.assert_called_once()
        decoder.assert_not_called()


if __name__ == "__main__":
    unittest.main(verbosity=2)
