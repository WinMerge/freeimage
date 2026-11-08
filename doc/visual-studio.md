# Build FreeImage with Visual Studio and CMake

Run these commands in PowerShell from the FreeImage source directory. CMake
builds FreeImage and its bundled dependencies together.

## Install the tools

- Install Visual Studio with **Desktop development with C++**, including an
  x86/x64 MSVC toolset and a Windows SDK. Build Tools also supports command-line
  builds; the Visual Studio IDE is needed to open the solution.
- Install CMake and add it to `PATH`. The presets require CMake 3.21 or newer;
  Visual Studio 2026 requires CMake 4.2 or newer.
- For ARM64 or ARM64EC, install the matching C++ ARM64 build tools. ARM64EC
  requires VS 2022 17.3 or newer. These binaries run on Windows ARM64.

The examples use VS 2022. For VS 2026, replace `vs2022` with `vs2026` in both
the preset names and directory names.

## Generate and build a single architecture

Each ordinary CMake solution has one fixed architecture. Use a separate build
directory for each target:

```powershell
# 32-bit x86
cmake --preset vs2022 -A Win32 -B build-vs2022-win32
cmake --build build-vs2022-win32 --config Release

# 64-bit x86
cmake --preset vs2022 -A x64 -B build-vs2022-x64
cmake --build build-vs2022-x64 --config Release

# Native ARM64
cmake --preset vs2022 -A ARM64 -B build-vs2022-arm64
cmake --build build-vs2022-arm64 --config Release

# ARM64EC
cmake --preset vs2022 -A ARM64EC -B build-vs2022-arm64ec
cmake --build build-vs2022-arm64ec --config Release
```

Open `FreeImage.sln` inside the selected build directory. Choose **Debug** or
**Release**, then **Build > Build Solution**. To build Debug from the command
line, use `--config Debug`.

Without a preset, the equivalent x64 configure command is:

```powershell
cmake -S . -B build-vs2022-x64 -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON
```

For VS 2026 the generator name is `Visual Studio 18 2026`. With a Visual Studio
generator, `--config` selects Debug/Release at build time; setting
`CMAKE_BUILD_TYPE` does not select the configuration.

## Generate one solution with a platform dropdown

```powershell
cmake --preset vs2022-all
```

Open **`build-vs2022-all/visualstudio/FreeImage.sln`**. The solution platform
dropdown offers **Win32**, **x64**, **ARM64**, **ARM64EC**, and **All Platforms**.
Choose the platform and Debug/Release, then build the solution. All Platforms
builds all four architectures, so all four toolsets must be installed.

To build all platforms from CMake:

```powershell
cmake --build build-vs2022-all --config Release --target FreeImageAllPlatforms
```

The combined solution uses Visual Studio Makefile projects that invoke CMake.
Keep CMake on `PATH` when building from the IDE. Libraries are placed under
`build-vs2022-all/visualstudio/artifacts/<architecture>/<configuration>/`.

Source and include references in this solution are relative. To share or move
it, keep the source tree and `visualstudio` directory in the same relative
locations on the same drive. Each machine/location receives fresh caches
under `visualstudio/cache/`. Ordinary CMake-generated solutions and caches are
specific to their original location; regenerate those after moving the tree.

The combined mode supports bundled dependencies and native MSVC toolchains.
For system codec libraries or external SDK toolchains, configure a separate
ordinary CMake build directory per architecture.

VS 2019 support is planned as a separate follow-up limited to Win32 and x64.
The combined presets in this change support VS 2022 and VS 2026.

## Choose static libraries, DLLs and codecs

Pass options when configuring; for example, to generate a combined DLL build:

```powershell
cmake --preset vs2022-all -DFREEIMAGE_STATIC=OFF
```

Use `-DFREEIMAGE_STATIC=ON` for a static `.lib`. The same setting applies to
every platform in the combined solution. To enable the optional modern codecs,
add `-DBUILD_LIBHEIF=ON -DBUILD_LIBJXL=ON` to the configure command. The matching
codec sources are bundled with this repository.

Use `-DFREEIMAGE_MSVC_RUNTIME=MD` for the dynamic C runtime or
`-DFREEIMAGE_MSVC_RUNTIME=MT` for the static C runtime. Consumers and prebuilt
dependencies should use a matching runtime setting.

## Run tests and install

The Visual Studio presets enable `BUILD_TESTS`. Run tests from an ordinary
Win32/x64 build directory whose binaries can execute on your machine:

```powershell
ctest --test-dir build-vs2022-x64 -C Release --output-on-failure
cmake --install build-vs2022-x64 --config Release --prefix install-vs2022-x64
```

Install each architecture into a separate prefix. Headers go under `include/`,
libraries under `lib/`, and DLLs under `bin/`.

If CMake reports a generator/platform mismatch, select a new build directory
for the new Visual Studio version or architecture. A configured directory
cannot switch architecture in place.
