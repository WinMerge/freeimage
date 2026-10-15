# Optional Python and NumPy bindings

Install a **shared** FreeImage build, then `pip install ./bindings/python`.
This package is independent of the core CMake build and does not bundle a
FreeImage binary or make Python a dependency of the C/C++ library. Set
`FREEIMAGE_LIBRARY` to the full library path if the system loader cannot find it.

```python
import freeimage_numpy as fi
image = fi.load("input.png")           # owned, top-down RGB/RGBA array
image[..., 0] = 255
fi.save("output.png", image)
encoded = fi.save_bytes(image, "PNG")
roundtrip = fi.load_bytes(encoded)
image, format_id = fi.loadf("input.png")
print(fi.version())
```

Supported arrays are H×W grayscale or H×W×{1,3,4}: native-endian uint8, uint16,
float32, and grayscale float64. Palettes are expanded to RGB/RGBA; ordinary
8-bit grayscale remains a 2D array. HDR values are not normalized. Indexed
transparency becomes RGBA. CMYK, complex pixels and header-only arrays are
rejected explicitly. Bitmap channel-order detection currently supports
little-endian hosts. Windows filenames use FreeImage's Unicode entry points.

Loading returns writable NumPy-owned memory, never a view of a freed bitmap.
Saving accepts sliced, reversed and noncontiguous arrays and does not modify
its input. Every per-operation bitmap and memory stream is released in a
finally block. The loaded library stays initialized for process lifetime.
Supported codecs and save types depend on the installed FreeImage build;
HEIF/AVIF and JPEG XL require those optional plugins to be enabled there.
Metadata import/export to Python objects is not part of this initial package.

This is an independent ctypes adaptation of FreeImageRe's Python/NumPy feature;
it does not copy their interpreter integration or replace the core C ABI.
It is provided under FreeImage's FIPL/GPLv2 dual license. NumPy and the installed
FreeImage codec libraries retain their own licenses. No package is published
to PyPI by building a wheel.

Tests: `python -m unittest discover -s bindings/python/tests -v` after installation.
