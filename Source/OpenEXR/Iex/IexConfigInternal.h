// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.

#pragma once

// Hardware FPU control-register / SIGFPE trap support is a niche,
// platform-specific feature (mostly Linux) not needed for EXR load/save -
// leaving all of these undefined falls back to IexMathFpu.cpp's portable
// no-op path.

/* #undef HAVE_UCONTEXT_H */
/* #undef IEX_HAVE_CONTROL_REGISTER_SUPPORT */
/* #undef IEX_HAVE_SIGCONTEXT_CONTROL_REGISTER_SUPPORT */
