#!/usr/bin/env python3
"""Build/extract deterministic, sparse test data without storing large binaries in git."""

import argparse
import gzip
import hashlib
import http.client
import io
import json
import os
from pathlib import Path, PurePosixPath
import re
import struct
import subprocess
import sys
import tempfile
import tarfile
import time
import urllib.error
import urllib.parse
import urllib.request
import zlib

DEFAULT_TAG = "test-fixtures-io64-v1"
RGB = [0x12, 0x34, 0x56]
MAX_LOGICAL_SIZE = 8 * 1024**3
README = r"""# FreeImage large-file I/O test fixtures

These are synthetic test inputs, not a FreeImage software release. All images
are generated from scratch and may be redistributed under CC0-1.0.

Each fixture has its own sparse .tar.gz release asset. manifest.json records
its filename, SHA-256, compressed size, logical size, intended decode/reject
result and expected pixel color. Download only the payload being tested.
Large TIFF files target the additive 64-bit input API in PR #197; the legacy
loading API and other viewers may not support those offsets.

The Python runner downloads/extracts one payload into a temporary directory,
runs the decoder, and removes that directory before advancing to the next
case. Failures and timeouts also trigger cleanup. Windows files are explicitly
marked sparse. Logical lengths above 4 GiB do not consume that much disk.

Example (replace DECODER with the future loader test executable):

    python scripts/tests/io64-fixtures.py run --manifest manifest.json \
        --release-url https://github.com/danoli3/FreeImage/releases/download/TAG \
        -- DECODER '{format}' '{expect}' '{path}'

Pin the manifest SHA-256 with --manifest-sha256 in CI. To extract a single
previously downloaded asset, use the extract subcommand with --manifest and
--output-dir. GNU tar and macOS bsdtar also understand these sparse archives.
Avoid tools that expand sparse holes when copying the extracted images.

Paths include spaces, Cyrillic, Greek, Arabic, Hindi, Chinese, Japanese,
Korean, combining characters and emoji, with valid and broken images in each.
Four malformed strip/offset cases return a bitmap on PR #197 head
6c964f35ab21b69fbfae4265e81c3f3df3bf8fd5. Their intended result remains
reject; known_behavior records the baseline. This corpus exposes those
failures without including loader fixes. No external/private images are used.
"""


def tiff(big=False, endian="<", ifd=16, pixel=1024, exif=False,
         width=1, height=1, strip_bytes=3):
    """Return sparse extents for a minimal uncompressed TIFF / BigTIFF."""
    pack = lambda fmt, *args: struct.pack(endian + fmt, *args)
    marker = b"II" if endian == "<" else b"MM"
    header = marker + (pack("HHHQ", 43, 8, 0, ifd) if big else pack("HI", 42, ifd))
    tags = [(256, 4, 1, pack("I", width)), (257, 4, 1, pack("I", height)),
            (258, 3, 3, pack("HHH", 8, 8, 8)), (259, 3, 1, pack("H", 1)),
            (262, 3, 1, pack("H", 2)),
            (273, 16 if big else 4, 1, pack("Q" if big else "I", pixel)),
            (277, 3, 1, pack("H", 3)), (278, 4, 1, pack("I", 1)),
            (279, 16 if big else 4, 1, pack("Q" if big else "I", strip_bytes)),
            (284, 3, 1, pack("H", 1))]
    extra = []
    if exif:
        if not big:
            raise ValueError("The metadata fixture uses BigTIFF")
        thumb = pixel + 512
        tags += [(330, 18, 1, pack("Q", thumb)),
                 (34665, 18, 1, pack("Q", pixel + 64))]
        text = b"ASCII\0\0\0IO64test\0"
        profile = (pack("Q", 2) + pack("HHQ", 36864, 7, 4) + b"0231\0\0\0\0" +
                   pack("HHQQ", 37510, 7, len(text), pixel + 128) + pack("Q", 0))
        extra += [(pixel + 64, profile), (pixel + 128, text)]
        thumbnail, _ = tiff(big=True, endian=endian, ifd=thumb, pixel=pixel + 1024)
        extra += thumbnail[1:]
    capacity = 8 if big else 4
    directory = pack("Q" if big else "H", len(tags))
    for tag, kind, count, data in sorted(tags):
        if len(data) > capacity:
            extra.append((ifd + 512, data))
            data = pack("I", ifd + 512)
        directory += pack("HHQ" if big else "HHI", tag, kind, count)
        directory += data.ljust(capacity, b"\0")
    directory += pack("Q" if big else "I", 0)
    regions = sorted([(0, header), (ifd, directory), (pixel, bytes(RGB))] + extra)
    return regions, max(offset + len(data) for offset, data in regions)


def png():
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 1, 1, 8, 2, 0, 0, 0)) +
            chunk(b"IDAT", zlib.compress(b"\0" + bytes(RGB), level=0)) + chunk(b"IEND", b""))


def bmp():
    return (b"BM" + struct.pack("<IHHI", 58, 0, 0, 54) +
            struct.pack("<IiiHHIIiiII", 40, 1, 1, 1, 24, 0, 4, 0, 0, 0, 0) +
            bytes(reversed(RGB)) + b"\0")


def fixtures():
    cases = []

    def add(name, fmt, expect, description, regions, size=None, **metadata):
        if isinstance(regions, bytes):
            regions = [(0, regions)] if regions else []
        if size is None:
            size = max((offset + len(data) for offset, data in regions), default=0)
        cases.append((dict(path=name, format=fmt, expect=expect, description=description,
                           logical_size=size, **metadata), regions))

    def image(name, **kwargs):
        regions, size = tiff(**kwargs)
        add("valid/" + name + ".tif", "TIFF", "decode", name, regions, size,
            width=1, height=1, rgb=RGB, **({"exif_user_comment": "IO64test", "thumbnail_rgb": RGB}
                                       if kwargs.get("exif") else {}))

    image("classic-small")
    image("classic-strip-above-2g", pixel=0x80001000)
    image("classic-strip-below-4g", pixel=0xfffff000)
    image("classic-big-endian-above-2g", endian=">", pixel=0x80001000)
    image("bigtiff-small", big=True)
    image("bigtiff-strip-above-4g", big=True, pixel=0x100001000)
    image("bigtiff-ifd-exif-thumbnail-above-4g", big=True,
          ifd=0x100000010, pixel=0x100001000, exif=True)
    image("bigtiff-big-endian-above-4g", big=True, endian=">",
          ifd=0x100000010, pixel=0x100001000)
    for name, data in {
        "empty": b"", "truncated-header": b"II\x2b\0\x08\0\0",
        "wrong-magic": b"II\xff\xff" + bytes(12),
        "invalid-offset-width": b"II" + struct.pack("<HHHQ", 43, 4, 0, 16),
        "ifd-past-eof": b"II" + struct.pack("<HHHQ", 43, 8, 0, 0x100000010),
        "ifd-outside-int64": b"II" + struct.pack("<HHHQ", 43, 8, 0, 0xffffffffffffffff),
        "truncated-ifd": b"II" + struct.pack("<HHHQ", 43, 8, 0, 16) + b"\x0a\0",
        "overflowing-ifd-count": b"II" + struct.pack("<HHHQ", 43, 8, 0, 16) + bytes([255]) * 8,
    }.items():
        add("broken/" + name + ".tif", "TIFF", "reject", name, data)
    for name, opts in [("zero-width", {"width": 0}), ("zero-height", {"height": 0})]:
        regions, size = tiff(big=True, **opts)
        add("broken/" + name + ".tif", "TIFF", "reject", name, regions, size)
    for name, big, ifd, pixel in [
        ("classic-truncated-strip-above-2g", False, 16, 0x80001000),
        ("bigtiff-truncated-strip-above-4g", True, 0x100000010, 0x100001000),
    ]:
        regions, _ = tiff(big=big, ifd=ifd, pixel=pixel)
        add("broken/" + name + ".tif", "TIFF", "reject", name,
            [(o, b[:2] if o == pixel else b) for o, b in regions], pixel + 2)
    regions, _ = tiff(big=True, pixel=0x100001000)
    add("broken/bigtiff-strip-past-eof.tif", "TIFF", "reject", "strip-past-eof", regions[:-1], 1024)
    regions, _ = tiff(big=True, ifd=0x100000010, pixel=0x100001000)
    add("broken/bigtiff-truncated-ifd-above-4g.tif", "TIFF", "reject", "truncated-ifd-above-4g",
        [(0, regions[0][1]), (0x100000010, b"\x0a\0")], 0x100000012)

    for fmt, data in [("PNG", png()), ("BMP", bmp())]:
        suffix = "." + fmt.lower()
        add("valid/small" + suffix, fmt, "decode", "non-TIFF fallback", data, width=1, height=1, rgb=RGB)
        for name, damaged in [("empty", b""), ("truncated-header", data[:7]),
                              ("truncated-pixels", data[:len(data)//2])]:
            add("broken/" + fmt.lower() + "-" + name + suffix, fmt, "reject", name, damaged)
    bad_bmp = bytearray(bmp()); struct.pack_into("<I", bad_bmp, 10, 0x80001000)
    add("broken/bmp-offset-past-eof.bmp", "BMP", "reject", "pixel offset above 2 GiB, past EOF", bytes(bad_bmp))
    names = ["spaces in name", "Кириллица", "Ελληνικά", "العربية", "हिन्दी",
             "中文", "日本語", "한국어", "combining-e\u0301", "emoji-\U0001f9ea"]
    small_tiff, small_size = tiff()
    for name in names:
        for fmt, data in [("TIFF", small_tiff), ("PNG", png()), ("BMP", bmp())]:
            suffix = ".tif" if fmt == "TIFF" else "." + fmt.lower()
            add("paths/" + name + "/valid image" + suffix, fmt, "decode",
                "nested multilingual filename", data, small_size if fmt == "TIFF" else None,
                width=1, height=1, rgb=RGB)
            add("paths/" + name + "/broken image" + suffix, fmt, "reject",
                "malformed input at a multilingual path", b"broken")
    known = {"broken/bigtiff-strip-past-eof.tif", "broken/bigtiff-truncated-strip-above-4g.tif",
             "broken/classic-truncated-strip-above-2g.tif", "broken/bmp-offset-past-eof.bmp"}
    for record, _ in cases:
        if record["path"] in known:
            record["known_behavior"] = dict(
                commit="6c964f35ab21b69fbfae4265e81c3f3df3bf8fd5", result="returns_bitmap",
                api="FreeImage_Load64" if record["format"] == "TIFF" else "FreeImage_Load",
                note="Malformed input is accepted; intended regression result is rejection.")
    return cases


def asset_name(path):
    label = re.sub(r"[^a-z0-9]+", "-", path.lower()).strip("-")[:80]
    return label + "-" + hashlib.sha256(path.encode("utf-8")).hexdigest()[:16] + ".tar.gz"


def sparse_blocks(regions, size):
    # GNU tar consumes complete 512-byte records between sparse extents.
    # Pad/merge the stored extents, leaving the logical file bytes unchanged.
    blocks = []
    for offset, data in regions:
        start = offset // 512 * 512
        end = min((offset + len(data) + 511) // 512 * 512, size)
        if blocks and start <= blocks[-1][1]:
            blocks[-1] = (blocks[-1][0], max(blocks[-1][1], end))
        else:
            blocks.append((start, end))
    stored = []
    for start, end in blocks:
        block = bytearray(end - start)
        for offset, data in regions:
            if start <= offset < end:
                block[offset - start:offset - start + len(data)] = data
        stored.append((start, bytes(block)))
    return stored


def build(output, tag=DEFAULT_TAG):
    if not re.fullmatch(r"test-fixtures-io64-v[1-9][0-9]*", tag):
        raise ValueError("Use a fixture tag such as test-fixtures-io64-v1, not a software version")
    output = Path(output)
    output.mkdir(parents=True, exist_ok=False)
    cases = sorted(fixtures(), key=lambda case: case[0]["path"])
    records = []
    for record, regions in cases:
        end = 0
        for offset, data in regions:
            if offset < end or offset + len(data) > record["logical_size"]:
                raise ValueError("Overlapping or out-of-bounds fixture extent")
            end = offset + len(data)
        record["extents"] = [dict(offset=o, length=len(b), sha256=hashlib.sha256(b).hexdigest())
                             for o, b in regions]
        stored = sparse_blocks(regions, record["logical_size"])
        name = asset_name(record["path"])
        with (output / name).open("wb") as raw:
            with gzip.GzipFile(fileobj=raw, mode="wb", filename="", mtime=0) as compressed:
                with tarfile.open(fileobj=compressed, mode="w", format=tarfile.PAX_FORMAT) as archive:
                    member = tarfile.TarInfo(record["path"])
                    member.size = sum(len(data) for _, data in stored)
                    sparse = [(offset, len(data)) for offset, data in stored] + [(record["logical_size"], 0)]
                    # GNU tar allocates the map while decoding numblocks, so
                    # numblocks must precede map in the ordered PAX headers.
                    member.pax_headers = {"GNU.sparse.size": str(record["logical_size"]),
                                          "GNU.sparse.numblocks": str(len(sparse)),
                                          "GNU.sparse.name": record["path"],
                                          "GNU.sparse.map": ",".join(str(n) for pair in sparse for n in pair)}
                    archive.addfile(member, io.BytesIO(b"".join(data for _, data in stored)))
        payload = (output / name).read_bytes()
        record["asset"] = dict(name=name, sha256=hashlib.sha256(payload).hexdigest(), size=len(payload))
        records.append(record)
    manifest = (json.dumps(dict(schema=1, tag=tag, license="CC0-1.0", fixtures=records),
                           ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    (output / "manifest.json").write_bytes(manifest)
    (output / "README.md").write_text(README, encoding="utf-8", newline="\n")
    names = sorted(record["asset"]["name"] for record in records) + ["manifest.json", "README.md"]
    sums = "".join(hashlib.sha256((output / name).read_bytes()).hexdigest() + "  " + name + "\n"
                   for name in names)
    (output / "SHA256SUMS").write_text(sums, encoding="ascii", newline="\n")
    return records


def mark_sparse(file):
    if os.name == "nt":
        import ctypes
        import msvcrt
        from ctypes import wintypes
        control = ctypes.WinDLL("kernel32", use_last_error=True).DeviceIoControl
        control.argtypes = [wintypes.HANDLE, wintypes.DWORD, wintypes.LPVOID, wintypes.DWORD,
                            wintypes.LPVOID, wintypes.DWORD, ctypes.POINTER(wintypes.DWORD), wintypes.LPVOID]
        control.restype = wintypes.BOOL
        returned = wintypes.DWORD()
        if not control(msvcrt.get_osfhandle(file.fileno()), 0x900c4, None, 0, None, 0,
                       ctypes.byref(returned), None):
            raise ctypes.WinError(ctypes.get_last_error())


def safe_path(root, name):
    path = PurePosixPath(name)
    if not name or path.is_absolute() or ".." in path.parts or "\\" in name or ":" in name:
        raise ValueError("Unsafe archive path: " + name)
    result = root.joinpath(*path.parts)
    result.resolve().relative_to(root.resolve())
    return result


def load_manifest(path, expected_sha256=None):
    payload = Path(path).read_bytes()
    if expected_sha256 and hashlib.sha256(payload).hexdigest() != expected_sha256:
        raise ValueError("Manifest SHA-256 mismatch")
    manifest = json.loads(payload)
    if manifest["schema"] != 1:
        raise ValueError("Unsupported manifest schema")
    return manifest["fixtures"]


def extract(archive_path, output, record):
    archive_path, output = Path(archive_path), Path(output)
    if hashlib.sha256(archive_path.read_bytes()).hexdigest() != record["asset"]["sha256"]:
        raise ValueError("Payload SHA-256 mismatch")
    # A fresh directory prevents collisions, symlink traversal and stale fixtures.
    output.mkdir(parents=True, exist_ok=False)
    members = 0
    with tarfile.open(archive_path, "r:gz") as archive:
        for member in archive:
            members += 1
            destination = safe_path(output, member.name)
            if not member.isfile() or member.size > MAX_LOGICAL_SIZE:
                raise ValueError("Unsupported archive member: " + member.name)
            if members != 1 or member.name != record["path"] or member.size != record["logical_size"]:
                raise ValueError("Payload does not match the selected fixture")
            regions = member.sparse if member.sparse is not None else [(0, member.size)]
            if sum(length for _, length in regions) > 1024**2:
                raise ValueError("Fixture payload exceeds 1 MiB")
            destination.parent.mkdir(parents=True, exist_ok=True)
            source = archive.extractfile(member)
            with destination.open("xb") as target:
                mark_sparse(target)
                for offset, length in regions:
                    if offset < 0 or length < 0 or offset + length > member.size:
                        raise ValueError("Invalid sparse extent")
                    if not length:
                        continue
                    source.seek(offset)
                    data = source.read(length)
                    if len(data) != length:
                        raise ValueError("Truncated sparse extent")
                    target.seek(offset)
                    target.write(data)
                target.truncate(member.size)
    if members != 1:
        raise ValueError("Missing fixture payload")
    path = safe_path(output, record["path"])
    with path.open("rb") as file:
        for extent in record["extents"]:
            file.seek(extent["offset"])
            if hashlib.sha256(file.read(extent["length"])).hexdigest() != extent["sha256"]:
                raise ValueError("Fixture payload mismatch: " + record["path"])
    return path


def download(url):
    """Retry transient transport failures, keeping each response size bounded."""
    for attempt in range(4):
        try:
            with urllib.request.urlopen(url, timeout=60) as response:
                payload = response.read(1024**2 + 1)
            if len(payload) > 1024**2:
                raise ValueError("Downloaded payload exceeds 1 MiB")
            return payload
        except (urllib.error.URLError, TimeoutError, ConnectionError, http.client.IncompleteRead) as error:
            if isinstance(error, urllib.error.HTTPError):
                retry = error.code in (408, 429, 500, 502, 503, 504)
                error.close()
                if not retry:
                    raise
            if attempt == 3:
                raise
            delay = 2**attempt
            print(f"Retrying download {url} in {delay}s (attempt {attempt + 2}/4): {error}",
                  file=sys.stderr, flush=True)
            time.sleep(delay)


def run_cases(records, command, payload_dir=None, release_url=None, timeout=30, temporary_root=None,
              allow_known_failures=False):
    if not command or bool(payload_dir) == bool(release_url):
        raise ValueError("Provide a decoder command and exactly one payload source")
    failed = 0
    known_failed = 0
    # The download and extraction both live inside this per-case scope. Keep
    # no payload cache: even failed decoders leave no images for the next case.
    for record in records:
        try:
            with tempfile.TemporaryDirectory(prefix="io64-case-", dir=temporary_root) as temporary:
                root = Path(temporary)
                name = record["asset"]["name"]
                if Path(name).name != name or not name.endswith(".tar.gz") or ":" in name or "\\" in name:
                    raise ValueError("Unsafe asset filename")
                if release_url:
                    archive = root / name
                    url = release_url.rstrip("/") + "/" + urllib.parse.quote(name)
                    archive.write_bytes(download(url))
                else:
                    archive = Path(payload_dir) / name
                path = extract(archive, root / "input", record)
                arguments = []
                for part in command:
                    for key, value in {"path": str(path), "format": record["format"],
                                       "expect": record["expect"],
                                       "known": record.get("known_behavior", {}).get("result", "none")}.items():
                        part = part.replace("{" + key + "}", value)
                    arguments.append(part)
                print("Testing " + record["path"], flush=True)
                try:
                    subprocess.run(arguments, check=True, timeout=timeout)
                except subprocess.CalledProcessError as error:
                    if allow_known_failures and error.returncode == 3 and "known_behavior" in record:
                        known_failed += 1
                        print("XFAIL " + record["path"] + ": documented malformed-input acceptance", flush=True)
                    else:
                        raise
        except (OSError, ValueError, http.client.IncompleteRead, tarfile.TarError, subprocess.SubprocessError) as error:
            failed += 1
            print("FAIL " + record["path"] + ": " + str(error), file=sys.stderr, flush=True)
    print(f"{len(records) - failed - known_failed} passed; {known_failed} known failures; "
          f"{failed} unexpected failures; all temporary payloads removed", flush=True)
    return failed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    make = commands.add_parser("build")
    make.add_argument("--output-dir", type=Path, required=True)
    make.add_argument("--tag", default=DEFAULT_TAG)
    unpack = commands.add_parser("extract")
    unpack.add_argument("archive", type=Path)
    unpack.add_argument("--output-dir", type=Path, required=True)
    run = commands.add_parser("run")
    source = run.add_mutually_exclusive_group(required=True)
    source.add_argument("--payload-dir", type=Path)
    source.add_argument("--release-url")
    run.add_argument("--timeout", type=float, default=30)
    run.add_argument("--allow-known-failures", action="store_true",
                     help="Report exit code 3 as XFAIL only for manifest-listed known behaviors")
    run.add_argument("--case", action="append", help="Select a manifest path; repeat for multiple cases")
    run.add_argument("decoder", nargs=argparse.REMAINDER)
    for command in (unpack, run):
        command.add_argument("--manifest", type=Path, required=True)
        command.add_argument("--manifest-sha256", help="Pin the reviewed manifest checksum in CI")
    args = parser.parse_args()
    if args.command == "build":
        records = build(args.output_dir, args.tag)
        print(f"{len(records)} independent payloads; {sum(r['logical_size'] for r in records):,} logical bytes")
        return 0
    records = load_manifest(args.manifest, args.manifest_sha256)
    if args.command == "extract":
        record = next(r for r in records if r["asset"]["name"] == args.archive.name)
        print(extract(args.archive, args.output_dir, record))
        return 0
    if args.case:
        requested = set(args.case)
        records = [r for r in records if r["path"] in requested]
        if {r["path"] for r in records} != requested:
            raise ValueError("Unknown fixture selected")
    decoder = args.decoder[1:] if args.decoder[:1] == ["--"] else args.decoder
    return 1 if run_cases(records, decoder, args.payload_dir, args.release_url, args.timeout,
                         allow_known_failures=args.allow_known_failures) else 0


if __name__ == "__main__":
    raise SystemExit(main())
