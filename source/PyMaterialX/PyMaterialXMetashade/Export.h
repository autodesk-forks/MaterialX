//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_METASHADE_EXPORT_H
#define MATERIALX_METASHADE_EXPORT_H

#include <MaterialXCore/Library.h>

/// @file
/// Macros for declaring imported and exported symbols.

#if defined(MATERIALX_BUILD_SHARED_LIBS)

#if defined(MATERIALX_METASHADE_EXPORTS)
    #define MX_METASHADE_API MATERIALX_SYMBOL_EXPORT
#else
    #define MX_METASHADE_API MATERIALX_SYMBOL_IMPORT
#endif

#else

#define MX_METASHADE_API

#endif

#endif
