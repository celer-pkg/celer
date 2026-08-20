# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from SPIRV-Cross.pro.

#####################################################################
## BundledSpirv_Cross Generic Library:
#####################################################################

qt_internal_add_3rdparty_library(BundledSpirv_Cross
    QMAKE_LIB_NAME spirv_cross
    SKIP_AUTOMOC
    STATIC
    EXCEPTIONS
    SOURCES
        ../3rdparty/SPIRV-Cross/spirv_cfg.cpp
        ../3rdparty/SPIRV-Cross/spirv_cpp.cpp
        ../3rdparty/SPIRV-Cross/spirv_cross.cpp
        ../3rdparty/SPIRV-Cross/spirv_cross_c.cpp
        ../3rdparty/SPIRV-Cross/spirv_cross_parsed_ir.cpp
        ../3rdparty/SPIRV-Cross/spirv_cross_util.cpp
        ../3rdparty/SPIRV-Cross/spirv_glsl.cpp
        ../3rdparty/SPIRV-Cross/spirv_hlsl.cpp
        ../3rdparty/SPIRV-Cross/spirv_msl.cpp
        ../3rdparty/SPIRV-Cross/spirv_parser.cpp
        ../3rdparty/SPIRV-Cross/spirv_reflect.cpp
    DEFINES
        SPIRV_CROSS_C_API_GLSL=1
        SPIRV_CROSS_C_API_HLSL=1
        SPIRV_CROSS_C_API_MSL=1
    ATTRIBUTION_FILE_DIR_PATHS
        ../3rdparty/SPIRV-Cross
)
qt_disable_warnings(BundledSpirv_Cross)
qt_set_symbol_visibility_hidden(BundledSpirv_Cross)
