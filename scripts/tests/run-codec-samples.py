#!/usr/bin/env python3
"""Run the display diagnostic on the pinned public archive linked in #198."""
import argparse
import hashlib
from pathlib import Path
import stat
import subprocess
import sys
import urllib.request
import zipfile

URL = "https://drive.google.com/uc?export=download&id=1BEvAC9NV9sIbS43byLwGeRllawLRlPtg"
SHA256 = "94f830c0aface6271cd3463eeb21dca547a6bd1b88dd833d9658556459bb21e7"
MAX_ARCHIVE = 32 * 1024 * 1024
EXPECTED = {"4px.jp2", "balloon.jp2", "sample1.jp2", "sample-photo.jp2",
            "cyberpunk2077.jxr", "panel-hdr.jxr"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, help="Use the saved archive instead of downloading")
    parser.add_argument("--tool", type=Path, required=True, help="CodecDiagnostics executable")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--previews", action="store_true", help="Also verify explicit RGB preview export")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    archive = args.archive
    if archive is None:
        archive = args.output / "samples.zip"
        request = urllib.request.Request(URL, headers={"User-Agent": "FreeImage-codec-check"})
        with urllib.request.urlopen(request, timeout=60) as response:
            data = response.read(MAX_ARCHIVE + 1)
        if len(data) > MAX_ARCHIVE:
            raise RuntimeError("sample download exceeded the expected archive size")
        archive.write_bytes(data)
    if archive.stat().st_size > MAX_ARCHIVE:
        raise RuntimeError("sample archive exceeded the expected size")
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != SHA256:
        raise RuntimeError("sample archive SHA-256 changed; this is an input/download failure, not a codec result")
    print("archive.sha256=" + digest, flush=True)
    destination = (args.output / "inputs").resolve()
    destination.mkdir(exist_ok=True)
    samples = {}
    with zipfile.ZipFile(archive) as zipped:
        for member in zipped.infolist():
            path = (destination / member.filename).resolve()
            if not path.is_relative_to(destination) or stat.S_ISLNK(member.external_attr >> 16):
                raise RuntimeError("unsafe sample archive member")
            if member.file_size > MAX_ARCHIVE:
                raise RuntimeError("unexpectedly large sample archive member")
            if member.is_dir():
                continue
            if path.suffix.lower() not in (".jp2", ".jxr"):
                continue
            if path.name in samples:
                raise RuntimeError("duplicate sample filename")
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(zipped.read(member))
            samples[path.name] = path
    if set(samples) != EXPECTED:
        raise RuntimeError("sample archive does not contain the six expected images")
    tool = args.tool.resolve()
    for name, sample in sorted(samples.items()):
        command = [str(tool), "--memory", str(sample)]
        if args.previews:
            command.append(str(args.output.resolve() / (name + ".png")))
        result = subprocess.run(command, capture_output=True, text=True, timeout=120)
        log = result.stdout + result.stderr
        (args.output / (name + ".log")).write_text(log, encoding="utf-8")
        print("\n--- " + name + " ---\n" + log, flush=True)
        if result.returncode:
            raise RuntimeError(name + ": diagnostic exit " + str(result.returncode))
        if "decode.file=success" not in log or "pixels_match=true" not in log:
            raise RuntimeError(name + ": missing successful file/memory verification")
        if name == "sample-photo.jp2" and "signature=J2K extension=JP2 selected=J2K" not in log:
            raise RuntimeError("mislabeled raw J2K was not selected by content")
        if args.previews and "preview=success" not in log:
            raise RuntimeError(name + ": preview verification missing")
    print("All six samples decoded from file and memory with matching pixels", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, zipfile.BadZipFile, subprocess.SubprocessError) as error:
        print("sample-check: " + str(error), file=sys.stderr)
        sys.exit(1)
