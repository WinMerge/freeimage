# HEIF and AVIF visual decoding example

These inputs reuse `TestAPI/sample.png` from this repository, adding a soft
circular alpha mask. The source RGBA PNG is encoded into HEIF/HEIC and AVIF
using libheif 1.23.2's `heif-enc -q 90` (with `-A` for AVIF).

The PNG previews below are created by **FreeImage_Load**, not by libheif's CLI
decoder. Both encoded files load as 240×215, 32-bit images through this PR's
plugins. `preview.cpp` composites the loaded pixels over a checkerboard and
exports a 2× PNG so GitHub and browsers can display the decoded result.

| Source PNG | HEIF loaded by FreeImage | AVIF loaded by FreeImage |
| --- | --- | --- |
| ![Source](source-preview.png) | ![HEIF decoded](heif-decoded.png) | ![AVIF decoded](avif-decoded.png) |

Reproduce with the FreeImage build from this PR installed under `/tmp/fi`:

```sh
cmake -S examples/heif-avif -B /tmp/fi-preview -DCMAKE_PREFIX_PATH=/tmp/fi
cmake --build /tmp/fi-preview
/tmp/fi-preview/HeifAvifPreview prepare TestAPI/sample.png examples/heif-avif/source-rgba.png
heif-enc -q 90 examples/heif-avif/source-rgba.png -o examples/heif-avif/example.heic
heif-enc -q 90 -A examples/heif-avif/source-rgba.png -o examples/heif-avif/example.avif
/tmp/fi-preview/HeifAvifPreview decode examples/heif-avif/source-rgba.png examples/heif-avif/source-preview.png
/tmp/fi-preview/HeifAvifPreview decode examples/heif-avif/example.heic examples/heif-avif/heif-decoded.png
/tmp/fi-preview/HeifAvifPreview decode examples/heif-avif/example.avif examples/heif-avif/avif-decoded.png
```

Configure the FreeImage library with `-DBUILD_LIBHEIF=ON` before installing.
Encoding is performed by the external CLI; this PR adds decoding only.
