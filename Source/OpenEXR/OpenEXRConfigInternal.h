// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.

#ifndef INCLUDED_OPENEXR_INTERNAL_CONFIG_H
#define INCLUDED_OPENEXR_INTERNAL_CONFIG_H 1

#pragma once

// Unlike upstream's CMake-substituted version of this file, these are
// computed here via the preprocessor at compile time (same approach as
// zlib's HAVE_UNISTD_H in this project's CMakeLists.txt) so the same
// header works correctly across this project's whole CI matrix (Linux/
// macOS/Windows) without needing per-platform generation.

#if defined(__linux__)
#define OPENEXR_IMF_HAVE_LINUX_PROCFS 1
#endif

#if defined(__APPLE__)
#define OPENEXR_IMF_HAVE_DARWIN 1
#endif

// Every compiler this project supports (C++14 minimum) has a complete
// <iomanip>, including std::right.
#define OPENEXR_IMF_HAVE_COMPLETE_IOMANIP 1

#if !defined(_WIN32) && !defined(_WIN64)
#define OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN 1
#endif

// Leave GCC AVX inline-asm and the ARM vld1q_f32_x2 shim off - both are
// narrow perf/compatibility opt-ins with safe portable fallbacks already
// in the code paths that check these macros. Note both are checked with
// #ifdef (not by value), so they must stay genuinely undefined rather than
// defined-as-0 - the latter would activate the ARM-specific asm workaround
// unconditionally, even on non-ARM builds.
/* #undef OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX */
/* #undef OPENEXR_MISSING_ARM_VLD1 */

#endif // INCLUDED_OPENEXR_INTERNAL_CONFIG_H
