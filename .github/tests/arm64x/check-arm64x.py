#!/usr/bin/env python3
"""Report whether a PE image is a real ARM64X binary.

dumpbin prints "(ARM64X)" for every image with hybrid CHPE metadata, which
includes plain ARM64EC images, so its header line cannot tell a merged DLL
from an unmerged one. A merged image is distinguished by its ARM64X dynamic
value relocations: the loader applies them to switch between the native
ARM64 view and the x64/ARM64EC view. This script reads them directly.
"""
import argparse
import struct
import sys
from pathlib import Path

MACHINES = {0x8664: "x64", 0xAA64: "ARM64", 0xA641: "ARM64EC", 0xA64E: "ARM64X", 0x14C: "x86"}
IMAGE_DYNAMIC_RELOCATION_ARM64X = 6


def sections(data, pe):
    count = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    base = pe + 24 + optional_size
    result = []
    for index in range(count):
        offset = base + 40 * index
        virtual_size, virtual_address, raw_size, raw_pointer = struct.unpack_from("<IIII", data, offset + 8)
        result.append((virtual_address, max(virtual_size, raw_size), raw_pointer))
    return result


def file_offset(table, rva):
    for virtual_address, size, raw_pointer in table:
        if virtual_address <= rva < virtual_address + size:
            return raw_pointer + rva - virtual_address
    raise ValueError(f"RVA {rva:#x} is not inside any section")


def arm64x_fixups(data, start, size):
    """Yield (rva, kind, value) for each ARM64X fixup in a base-relocation-style block list."""
    block = start
    while block < start + size:
        page, block_size = struct.unpack_from("<II", data, block)
        if block_size < 8:
            break
        entry = block + 8
        while entry < block + block_size:
            word = struct.unpack_from("<H", data, entry)[0]
            entry += 2
            if word == 0:
                continue
            rva, kind, meta = page + (word & 0xFFF), (word >> 12) & 3, (word >> 14) & 3
            if kind == 0:
                yield rva, "zero", 1 << meta
            elif kind == 1:
                width = 1 << meta
                yield rva, "value", int.from_bytes(data[entry:entry + width], "little")
                entry += width
            elif kind == 2:
                yield rva, "delta", struct.unpack_from("<H", data, entry)[0]
                entry += 2
        block += block_size


def inspect(path):
    data = Path(path).read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("not a PE image")
    machine = struct.unpack_from("<H", data, pe + 4)[0]
    optional = pe + 24
    if struct.unpack_from("<H", data, optional)[0] != 0x20B:
        raise ValueError("not a PE32+ image")
    table = sections(data, pe)
    report = {"machine": MACHINES.get(machine, hex(machine)), "chpe": False,
              "dvrt_version": None, "arm64x_fixups": 0, "machine_fixup": None}
    load_config_rva, load_config_size = struct.unpack_from("<II", data, optional + 112 + 10 * 8)
    if not load_config_rva:
        return report
    load_config = file_offset(table, load_config_rva)
    declared = struct.unpack_from("<I", data, load_config)[0]
    if declared >= 0xD0:
        report["chpe"] = struct.unpack_from("<Q", data, load_config + 0xC8)[0] != 0
    if declared < 0xE6:
        return report
    dvrt_offset, dvrt_section = struct.unpack_from("<IH", data, load_config + 0xE0)
    if not dvrt_section:
        return report
    _, _, raw_pointer = table[dvrt_section - 1]
    dvrt = raw_pointer + dvrt_offset
    version, size = struct.unpack_from("<II", data, dvrt)
    report["dvrt_version"] = version
    if version != 1:
        return report
    entry, end = dvrt + 8, dvrt + 8 + size
    machine_field = pe + 4
    while entry < end:
        symbol, block_size = struct.unpack_from("<QI", data, entry)
        entry += 12
        if symbol == IMAGE_DYNAMIC_RELOCATION_ARM64X:
            for rva, kind, value in arm64x_fixups(data, entry, block_size):
                report["arm64x_fixups"] += 1
                # The headers are mapped at RVA 0, so the Machine field's RVA is its file offset.
                if rva == machine_field and kind == "value":
                    report["machine_fixup"] = MACHINES.get(value, hex(value))
        entry += block_size
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("images", nargs="+")
    parser.add_argument("--require-arm64x", action="store_true",
                        help="exit 1 unless every image carries ARM64X fixups")
    arguments = parser.parse_args()
    failed = False
    for image in arguments.images:
        report = inspect(image)
        merged = report["arm64x_fixups"] > 0
        print(f"{image}: machine={report['machine']} chpe={report['chpe']} "
              f"dvrt_version={report['dvrt_version']} arm64x_fixups={report['arm64x_fixups']} "
              f"machine_fixup={report['machine_fixup']} -> {'ARM64X' if merged else 'not ARM64X'}",
              flush=True)
        failed |= arguments.require_arm64x and not merged
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
