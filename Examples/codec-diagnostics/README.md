# Codec loading and preview diagnostics

This tool separates format detection, native decoding, memory-I/O verification,
and display-preview conversion. It was added to investigate issue #198.

Build from this source tree:

```sh
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_JXR=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target CodecDiagnostics --config Debug --parallel
build/TestAPI/CodecDiagnostics --memory image.jxr preview.png
```

For Visual Studio, the executable is `build/TestAPI/Debug/CodecDiagnostics.exe`.
Alternatively, install FreeImage and configure this directory using
`-DCMAKE_PREFIX_PATH=/path/to/freeimage-install`. For DLL installs, include the
installation's `bin` directory in `PATH` when running that standalone build.

The input signature takes priority over the extension. A raw J2K codestream
named `.jp2` therefore uses the J2K decoder. An unknown signature falls back
to the filename; missing codec support is reported separately from decoding.
`--memory` additionally compares file and memory decodes, including pixel hashes.
It uses the existing DWORD-sized memory-stream API, not the proposed large-file API.

The optional output is an explicit **24-bit RGB display preview**. RGB16/RGBA16
images are reduced to 8-bit channels. RGBF/RGBAF images use Drago HDR tone mapping.
This changes HDR appearance and discards alpha in the preview; native pixels
remain unchanged. Preserve the native image for HDR processing and choose an
appropriate tone-mapping/alpha policy in your application. Unsupported preview
types fail at the conversion stage without being reported as decode failures.

The supplied #198 archive includes a raw J2K file named `sample-photo.jp2`, a
`FIT_RGB16` JP2, and two JXR files decoded as `FIT_RGBAF`. Direct
`FreeImage_ConvertTo24Bits`, `FreeImage_ConvertTo32Bits`, or
`FreeImage_ConvertToStandardType` calls are not interchangeable for these types.
The .NET `GetBitmap` wrapper requires `FIT_BITMAP`; the C++ FreeImagePlus loader
can retain native typed images. A caller must distinguish decoding from conversion.

`CodecDisplayTest` uses synthetic images for these paths. The `JP2/JXR sample
checks` action also exercises the pinned archive linked in #198 on Windows,
without committing the supplied photographs to the repository. It reports decode
and preview results in the job log. The same archive can be checked locally:

```sh
python3 scripts/tests/run-codec-samples.py --archive /path/to/samples.zip \
  --tool build/TestAPI/CodecDiagnostics --output /tmp/codec-samples --previews
```

If `--archive` is omitted, the script downloads the public archive linked in #198
and checks its SHA-256 before extraction.
