# Experimental Windows ARM64X DLL validation

The `Windows ARM64X experiment` workflow builds FreeImage twice using MSVC:
ARM64 and ARM64EC, with matching C++17 and static CRT settings. It captures the
native linker inputs with `/LINKREPROFULLPATHRSP`, then relinks the EC objects
with those native objects/libraries using `/MACHINE:ARM64X`. The original two
DLLs are retained as runtime controls. This creates a merged binary, rather
than a forwarder that depends on two additional FreeImage DLLs.

Debug and Release artifacts run on `windows-11-arm`. Separate ARM64, ARM64EC
and x64 test programs dynamically load the exact same hybrid DLL. The programs
exercise public exports, allocation, pixels and memory/custom-I/O round trips
for PNG, TIFF, WebP, JP2, JXR and JPEG. Lossless formats must retain the source
pixels; all formats must decode identically through the two input APIs.
The custom reader follows `fread` semantics at EOF: it copies a partial final
item while returning only the number of complete items. JXR needs those bytes
when a packet read extends beyond the end of a small encoded image.
Unmerged native/EC controls must work in matching consumers and fail to load
in the incompatible consumer. SHA-256 checks ensure downloaded artifacts and
the shared DLL are unchanged.
Every consumer/DLL pairing runs even if an earlier probe fails; any failed
pairing still fails the runtime job.

The wrapper CMake project leaves the production FreeImage build unchanged.
Default bundled libraries, including OpenEXR, LibRaw and JXR, are compiled.
Optional HEIF/JPEG XL bundles are outside this initial experiment. Windows
ARM64 runtime checks are required: an x64-hosted cross-build alone cannot
validate the ARM64 loader's two personalities.

MSVC from VS 2022 17.11 or newer is required. To reproduce on a Windows x64
build host with the ARM64/ARM64EC toolsets installed:

```powershell
./.github/tests/arm64x/build.ps1 -Configuration Release `
  -BuildRoot "$env:TEMP/arm64x-build" -OutputRoot "$env:TEMP/arm64x-artifacts"
```

Copy `arm64x-artifacts` to a Windows ARM64 system, then run:

```powershell
./.github/tests/arm64x/run.ps1 -Artifacts ./arm64x-artifacts
```

GitHub Actions uploads the DLLs, consumers, PE/export reports and checksums as
short-lived test artifacts. They are not release packages. The CMake approach
follows [Microsoft's ARM64X build documentation](https://learn.microsoft.com/en-us/windows/arm/arm64x-build).
