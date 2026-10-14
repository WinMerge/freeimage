What is FreeImage ?
-----------------------------------------------------------------------------
FreeImage is an Open Source library project for developers who would like to support popular graphics image formats like PNG, BMP, JPEG, TIFF and others as needed by today's multimedia applications.
FreeImage is easy to use, fast, multithreading safe, and cross-platform (works with Windows, Linux and Mac OS X).

### This GitHub fork

Security and bug fixes on top of upstream FreeImage, plus a CMake build that compiles FreeImage and every bundled library from one `CMakeLists.txt`. This repository is the source tree. GitHub releases are tags, not prebuilt DLLs. Build a static library or a DLL with the commands in [Building this fork](#building-this-fork).

This tree is FreeImage **3.19.22**. `FreeImage_GetVersion()` prints `FREEIMAGE_MAJOR_VERSION`, `FREEIMAGE_MINOR_VERSION`, and `FREEIMAGE_RELEASE_SERIAL` from `Source/FreeImage.h`. CMake takes the version from the latest numeric git tag (`git describe`) and falls back to 3.19.22 when the archive has no `.git` history.

The ANSI C API is usable from C, C++, VB, C#, Delphi, Java, and from scripting languages such as Perl, Python, PHP, TCL, Lua, and Ruby.

### Why use FreeImage instead of linking each format library yourself?

Loading a PNG, a GIF and a JPEG from scratch in C++ means three different APIs (`libpng`, `giflib`-or-hand-rolled-LZW, `libjpeg`), three different error-handling conventions, and three sets of build flags to get right across platforms. FreeImage wraps [all of the libraries below](#supported-formats--bundled-libraries) behind one API - one `FreeImage_Load()`/`FreeImage_Save()` pair, one pixel format model (`FIBITMAP`), one metadata API (EXIF/IPTC/XMP) - so format-specific code doesn't leak into the rest of your application. Swapping a PNG for a WebP is a one-line change rather than a new dependency. Camera RAW uses the same `FreeImage_Load()` call. It is included unless the build passes `-DBUILD_LIBRAWLITE=OFF`.

This fork additionally builds the whole stack (FreeImage plus every bundled library) from one `CMakeLists.txt`, so `find_package(FreeImage)` is the only thing a consuming CMake project needs - see [Using compiled binaries](#using-compiled-binaries) below.

## Supported formats & bundled libraries

Formats implemented directly in FreeImage's own plugin code (no external library): BMP, ICO, TARGA/TGA, PCX, DDS, GIF, PSD, PICT, PFM, RAS/Sun Raster, SGI, XBM, XPM, Amiga IFF/LBM, Kodak PCD, PNM/PBM/PGM/PPM, CUT, and WBMP.

Everything else is backed by a bundled copy of the format's reference library. Turn a format off with its `BUILD_*` option, or link a system copy with `USE_SYSTEM_*` (or `FREEIMAGE_USE_SYSTEM_LIBS`, which turns every `USE_SYSTEM_*` default on). See `CMakeLists.txt`.

The bundled libtiff is fully synced to 4.7.2, including its integer-overflow hardening and decompression-ratio checks. OpenEXR is included when `CMAKE_CXX_STANDARD` is unset or 17 or newer, and left out for an older standard.

| Formats | Library | Bundled version | Default |
|---|---|---|---|
| PNG | [libpng](http://www.libpng.org/pub/png/libpng.html) (+ [zlib](https://zlib.net/)) | 1.6.58 (zlib 1.3.2) | on |
| JPEG | [libjpeg (IJG)](http://ijg.org/) | 10 (25 Jan 2026) | on |
| TIFF | [libtiff](http://www.libtiff.org/) | 4.7.2 | on |
| JPEG 2000 (J2K/JP2) | [OpenJPEG](https://github.com/uclouvain/openjpeg) | 2.5.4 | on |
| OpenEXR (HDR) | [OpenEXR](https://openexr.com/) (+ [Imath](https://github.com/AcademySoftwareFoundation/Imath), [libdeflate](https://github.com/ebiggers/libdeflate)) | 3.5.1 (Imath 3.2.3, libdeflate 1.26, Zstandard 1.5.7, OpenJPH 0.32.0) | on for C++17+ |
| WebP | [libwebp](https://developers.google.com/speed/webp) | 1.6.0 | on |
| Camera RAW | [LibRaw](https://www.libraw.org/) | 0.22.2 | on |
| JPEG-XR | [jxrlib](https://github.com/4creators/jxrlib) | unversioned snapshot | Windows only |

## Original Source Code Upstream
https://sourceforge.net/projects/freeimage
Original library can be found : https://freeimage.sourceforge.io 

## Status

[![CMake build](https://github.com/danoli3/FreeImage/actions/workflows/cmake.yml/badge.svg)](https://github.com/danoli3/FreeImage/actions/workflows/cmake.yml)

## Building this fork

JPEG-XR is included only on Windows (`BUILD_JXR` defaults to `WIN32`). Camera RAW is included by default (`-DBUILD_LIBRAWLITE=OFF` leaves it out).

### Simply build it

```bash
cmake . -B cmake-build
cmake --build cmake-build # On Linux, add -j$(nproc) for multicore build
```

### Platform-specific examples

**Linux (gcc) / macOS (clang)**
```bash
cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -- -j$(nproc)   # macOS: -j$(sysctl -n hw.ncpu)
```

**Windows (MSVC, Visual Studio toolchain)**
```bash
cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_CXX_STANDARD=17
cmake --build build --config Debug
```
See [Generate Visual Studio project files](#generate-visual-studio-project-files) below for opening the solution directly in the IDE.

**Windows (MinGW, e.g. a toolchain already on `PATH`)**
```bash
cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
cmake --build build
```

**Windows (MinGW-w64 via [MSYS2](https://www.msys2.org/), see [#30](https://github.com/danoli3/FreeImage/issues/30))**

From an MSYS2 `MINGW64` shell:
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**Plain `make` (Linux, no CMake)**
```bash
make -j$(nproc)
```

### Generate Visual Studio project files

```bash
cmake --preset vs2022   # or: cmake --preset vs2026
```
Generates a solution under `build-vs2022/` (or `build-vs2026/`). Open `FreeImage.sln` from there. Both presets are Windows-only and turn `BUILD_TESTS` on. `vs2026` needs CMake 4.2 or newer.

`cmake --preset ninja-msvc` builds with `cl.exe` and Ninja under `build-ninja-msvc/`. Run it from a Developer Command Prompt, or after `vcvarsall.bat`, so `cl.exe` is on `PATH`. See `CMakePresets.json`.

### Build and install Debug and Release configuration

```bash
cmake . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install_dir
cmake --build cmake-build-debug --config Debug --target install # Linux: -j$(nproc)

cmake . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install_dir
cmake --build cmake-build-release --config Release --target install # Linux: -j$(nproc)
```
Now `install_dir` contains the compiled binaries for debug and release, as well as the header file and the CMake Config files.

## Using compiled binaries

First, build and install it like explained above.

Then, in another project add the following CMake code:
```cmake
set(CMAKE_PREFIX_PATH <freeimage_install_location>)
find_package(FreeImage CONFIG REQUIRED)
...
target_link_libraries(<your_target> PRIVATE FreeImage::FreeImage)
```

For `find_package` to work, simply set `CMAKE_PREFIX_PATH` to the directory where the compiled binaries are installed (the `install_dir` folder from above).

## Running tests

```bash
cmake . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build cmake-build-debug --config Debug # Linux: -j$(nproc)
ctest --test-dir cmake-build-debug -C Debug # Optionally --rerun-failed --output-on-failure
```

> This ctest command only works with CMake 3.20 or higher. For earlier versions, you must `cd` into `cmake-build-debug` and call ctest without `--test-dir cmake-build-debug`.
