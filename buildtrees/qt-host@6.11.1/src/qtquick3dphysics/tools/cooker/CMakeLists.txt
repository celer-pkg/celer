# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(target_name cooker)
qt_internal_add_app(${target_name}
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Quick3D
        Qt::Quick3DPrivate
        Qt::Quick3DPhysics
        Qt::Quick3DPhysicsPrivate
        Qt::BundledPhysX
)
target_include_directories(${target_name} SYSTEM
    PRIVATE
    ../../src/3rdparty/PhysX/include
    ../../src/3rdparty/PhysX/pxshared/include
)
if (UNIX OR MINGW)
    # Needed for PxPreprocessor.h error
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        qt_internal_extend_target(${target_name} DEFINES _DEBUG)
    else()
        qt_internal_extend_target(${target_name} DEFINES NDEBUG)
    endif()
endif()
qt_internal_extend_target(${target_name} DEFINES PX_PHYSX_STATIC_LIB)
