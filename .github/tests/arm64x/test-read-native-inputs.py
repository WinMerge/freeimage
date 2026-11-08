#!/usr/bin/env python3
"""Exercise linker response encodings before paying for the ARM64 builds."""
from pathlib import Path
import subprocess
import tempfile


parser = Path(__file__).with_name("ReadNativeInputs.cmake").resolve()
with tempfile.TemporaryDirectory(prefix="arm64x-response-") as temporary:
    root = Path(temporary)
    paths = [root / "native object.obj", root / "native codec.lib", root / "exports file.def"]
    for path in paths:
        path.touch()
    obj, lib, definition = [path.as_posix() for path in paths]
    arguments = (f'"{obj}"\r\n"{lib}"\r\n/DEF:"{definition}"\r\n'
                 '/OUT:"ignored output.dll" /DEFAULTLIB:ignored.lib /MACHINE:ARM64\r\n')
    for encoding in ("utf-8", "utf-8-sig", "utf-16", "utf-16-be"):
        response = root / "native.rsp"
        data = arguments.encode(encoding)
        if encoding == "utf-16-be":
            data = b"\xfe\xff" + data
        response.write_bytes(data)
        script = root / "check.cmake"
        script.write_text(
            f'include("{parser.as_posix()}")\n'
            f'arm64x_read_native_inputs("{response.as_posix()}" objects libraries definition)\n'
            f'if(NOT objects STREQUAL "{obj}" OR NOT libraries STREQUAL "{lib}" '
            f'OR NOT definition STREQUAL "{definition}")\n'
            '  message(FATAL_ERROR "Native linker inputs changed or unwanted flags were retained")\n'
            'endif()\n', encoding="utf-8")
        subprocess.run(["cmake", "-P", str(script)], check=True)
        print(f"Native linker inputs: {encoding} passed", flush=True)
