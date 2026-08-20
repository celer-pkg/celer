# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from utils.pro.

#####################################################################
## Quick3DUtils Module:
#####################################################################

qt_internal_add_module(Quick3DUtils
    SOURCES
        qqsbcollection.cpp qqsbcollection_p.h
        qssgbounds3.cpp qssgbounds3_p.h
        qssgdataref.cpp qssgdataref_p.h
        qssgfrustum_p.h
        qssginvasivelinkedlist_p.h
        qssgmeshbvh.cpp qssgmeshbvh_p.h
        qssgplane.cpp qssgplane_p.h
        qssgrenderbasetypes.cpp qssgrenderbasetypes_p.h qssgrenderbasetypes.h
        qssgutils.cpp qssgutils_p.h
        qssgmeshbvhbuilder.cpp qssgmeshbvhbuilder_p.h
        qssgmesh.cpp qssgmesh_p.h
        qssgassert.cpp qssgassert_p.h
        qssgaosettings_p.h
        qtquick3dutilsglobal_p.h
        qquick3dprofiler.cpp
        qquick3dprofiler_p.h
        ../3rdparty/xatlas/xatlas.cpp ../3rdparty/xatlas/xatlas.h
        qssglightmapuvgenerator.cpp qssglightmapuvgenerator_p.h
        ../3rdparty/meshoptimizer/src/allocator.cpp
        ../3rdparty/meshoptimizer/src/clusterizer.cpp
        ../3rdparty/meshoptimizer/src/indexcodec.cpp
        ../3rdparty/meshoptimizer/src/indexgenerator.cpp
        ../3rdparty/meshoptimizer/src/meshoptimizer.h
        ../3rdparty/meshoptimizer/src/overdrawanalyzer.cpp
        ../3rdparty/meshoptimizer/src/overdrawoptimizer.cpp
        ../3rdparty/meshoptimizer/src/simplifier.cpp
        ../3rdparty/meshoptimizer/src/spatialorder.cpp
        ../3rdparty/meshoptimizer/src/stripifier.cpp
        ../3rdparty/meshoptimizer/src/vcacheanalyzer.cpp
        ../3rdparty/meshoptimizer/src/vcacheoptimizer.cpp
        ../3rdparty/meshoptimizer/src/vertexcodec.cpp
        ../3rdparty/meshoptimizer/src/vertexfilter.cpp
        ../3rdparty/meshoptimizer/src/vfetchanalyzer.cpp
        ../3rdparty/meshoptimizer/src/vfetchoptimizer.cpp
        ../3rdparty/meshoptimizer/src/partition.cpp
        ../3rdparty/meshoptimizer/src/quantization.cpp
    NO_UNITY_BUILD_SOURCES
        ../3rdparty/xatlas/xatlas.cpp ../3rdparty/xatlas/xatlas.h
        ../3rdparty/meshoptimizer/src/allocator.cpp
        ../3rdparty/meshoptimizer/src/clusterizer.cpp
        ../3rdparty/meshoptimizer/src/indexcodec.cpp
        ../3rdparty/meshoptimizer/src/indexgenerator.cpp
        ../3rdparty/meshoptimizer/src/meshoptimizer.h
        ../3rdparty/meshoptimizer/src/overdrawanalyzer.cpp
        ../3rdparty/meshoptimizer/src/overdrawoptimizer.cpp
        ../3rdparty/meshoptimizer/src/simplifier.cpp
        ../3rdparty/meshoptimizer/src/spatialorder.cpp
        ../3rdparty/meshoptimizer/src/stripifier.cpp
        ../3rdparty/meshoptimizer/src/vcacheanalyzer.cpp
        ../3rdparty/meshoptimizer/src/vcacheoptimizer.cpp
        ../3rdparty/meshoptimizer/src/vertexcodec.cpp
        ../3rdparty/meshoptimizer/src/vertexfilter.cpp
        ../3rdparty/meshoptimizer/src/vfetchanalyzer.cpp
        ../3rdparty/meshoptimizer/src/vfetchoptimizer.cpp
        ../3rdparty/meshoptimizer/src/partition.cpp
        ../3rdparty/meshoptimizer/src/quantization.cpp
    DEFINES
        QT_BUILD_QUICK3DUTILS_LIB
    INCLUDE_DIRECTORIES
        ../3rdparty/xatlas
        ../3rdparty/meshoptimizer/src/
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QuickPrivate
    SSG_HEADER_FILTERS
        "(^|/)qssg.*[^_][^p]\\.h$"
    ATTRIBUTION_FILE_DIR_PATHS
        ../3rdparty/xatlas
        ../3rdparty/meshoptimizer
)

# Silence warnings in 3rdparty code
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set_source_files_properties(../3rdparty/xatlas/xatlas.cpp PROPERTIES COMPILE_FLAGS "-Wno-type-limits")
endif()

#### Keys ignored in scope 1:.:.:utils.pro:<TRUE>:
# MODULE = "quick3dutils"
