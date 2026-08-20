# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from glslang.pro.

#####################################################################
## BundledGlslang_Spirv Generic Library:
#####################################################################

qt_internal_add_3rdparty_library(BundledGlslang_Spirv
    QMAKE_LIB_NAME glslang_spirv
    STATIC
    SKIP_AUTOMOC
    SOURCES
        ../3rdparty/glslang/SPIRV/GlslangToSpv.cpp
        ../3rdparty/glslang/SPIRV/InReadableOrder.cpp
        ../3rdparty/glslang/SPIRV/Logger.cpp
        ../3rdparty/glslang/SPIRV/SPVRemapper.cpp
        ../3rdparty/glslang/SPIRV/SpvBuilder.cpp
        ../3rdparty/glslang/SPIRV/SpvPostProcess.cpp
        ../3rdparty/glslang/SPIRV/SpvTools.cpp
        ../3rdparty/glslang/SPIRV/disassemble.cpp
        ../3rdparty/glslang/SPIRV/doc.cpp
    INCLUDE_DIRECTORIES
        ../3rdparty/glslang
    PUBLIC_LIBRARIES
        # Remove: core
        # Remove: gui
    ATTRIBUTION_FILE_DIR_PATHS
        ../3rdparty/glslang
)
qt_disable_warnings(BundledGlslang_Spirv)
qt_set_symbol_visibility_hidden(BundledGlslang_Spirv)

## Scopes:
#####################################################################

qt_internal_extend_target(BundledGlslang_Spirv CONDITION WIN32
    DEFINES
        GLSLANG_OSINCLUDE_WIN32
)

qt_internal_extend_target(BundledGlslang_Spirv CONDITION UNIX
    DEFINES
        GLSLANG_OSINCLUDE_UNIX
)
#####################################################################
## BundledGlslang_Osdependent Generic Library:
#####################################################################

# special case begin
if(WIN32)
    set(GLSLANG_OSDEP_PATH ../3rdparty/glslang/glslang/OSDependent/Windows)
elseif(UNIX)
    set(GLSLANG_OSDEP_PATH ../3rdparty/glslang/glslang/OSDependent/Unix)
endif()
# special case end

qt_internal_add_3rdparty_library(BundledGlslang_Osdependent
    QMAKE_LIB_NAME glslang_osdependent
    STATIC
    SKIP_AUTOMOC
    SOURCES
        ${GLSLANG_OSDEP_PATH}/ossource.cpp # special case
    INCLUDE_DIRECTORIES
        ../3rdparty/glslang
    PUBLIC_LIBRARIES
        # Remove: core
        # Remove: gui
    ATTRIBUTION_FILE_DIR_PATHS
        ../3rdparty/glslang
)
qt_disable_warnings(BundledGlslang_Osdependent)
qt_set_symbol_visibility_hidden(BundledGlslang_Osdependent)

## Scopes:
#####################################################################

qt_internal_extend_target(BundledGlslang_Osdependent CONDITION WIN32
    DEFINES
        GLSLANG_OSINCLUDE_WIN32
)

#### Keys ignored in scope 7:.:.:glslang-osdependent.pro:WIN32:
# GLSLANG_OSDEP_PATH = "$$GLSLANG_PATH/glslang/OSDependent/Windows"

qt_internal_extend_target(BundledGlslang_Osdependent CONDITION UNIX
    DEFINES
        GLSLANG_OSINCLUDE_UNIX
)

#### Keys ignored in scope 8:.:.:glslang-osdependent.pro:UNIX:
# GLSLANG_OSDEP_PATH = "$$GLSLANG_PATH/glslang/OSDependent/Unix"

qt_internal_extend_target(BundledGlslang_Osdependent CONDITION LINUX
    PUBLIC_LIBRARIES
        pthread
)

#####################################################################
## BundledGlslang_Glslang Generic Library:
#####################################################################

qt_internal_add_3rdparty_library(BundledGlslang_Glslang
    QMAKE_LIB_NAME glslang_glslang
    STATIC
    SKIP_AUTOMOC
    SOURCES
        ../3rdparty/glslang/glslang/GenericCodeGen/CodeGen.cpp
        ../3rdparty/glslang/glslang/GenericCodeGen/Link.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/Constant.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/InfoSink.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/Initialize.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/IntermTraverse.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/Intermediate.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/ParseContextBase.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/ParseHelper.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/PoolAlloc.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/RemoveTree.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/Scan.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/ShaderLang.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/SymbolTable.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/Versions.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/attribute.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/glslang_tab.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/intermOut.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/iomapper.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/limits.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/linkValidate.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/parseConst.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/propagateNoContraction.cpp
        ../3rdparty/glslang/glslang/MachineIndependent/reflection.cpp
        ../3rdparty/glslang/glslang/ResourceLimits/ResourceLimits.cpp
    INCLUDE_DIRECTORIES
        ../3rdparty/glslang
    PUBLIC_LIBRARIES
        # Remove: core
        # Remove: gui
    ATTRIBUTION_FILE_DIR_PATHS
        ../3rdparty/glslang
)
qt_disable_warnings(BundledGlslang_Glslang)
qt_set_symbol_visibility_hidden(BundledGlslang_Glslang)

set_target_properties(BundledGlslang_Glslang PROPERTIES UNITY_BUILD OFF)

## Scopes:
#####################################################################

qt_internal_extend_target(BundledGlslang_Glslang CONDITION WIN32
    DEFINES
        GLSLANG_OSINCLUDE_WIN32
)

qt_internal_extend_target(BundledGlslang_Glslang CONDITION UNIX
    DEFINES
        GLSLANG_OSINCLUDE_UNIX
)
