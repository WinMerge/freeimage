# Dependency diagnostics

GetDependenciesCount and GetDependencyInfo expose the libraries linked into this
build through an additive C ABI inspired by FreeImageRe. FreeImage_GetVersion
retains its meaning. FI_DEPENDENCY_INFO contains name, compiled_version,
runtime_version and system_library. Strings are borrowed, immutable, and valid
for library lifetime. Null versions mean unknown, not version 0. JPEG reports
its API level explicitly; TIFF returns a multiline version/copyright string.
OpenEXR/Imath currently report compile-time versions only. System dependencies
are never identified with the bundled headers' versions.

The list describes linked build dependencies, not private libraries of external
plugins or a user's plugin-enable toggles. Combine it with GetFIFCount and
FIFSupportsReading/Writing to inspect format support. Out-of-range indices and
null output pointers fail without changing the output structure. The API does
not require FreeImage_Initialise or transfer any bitmap ownership.
