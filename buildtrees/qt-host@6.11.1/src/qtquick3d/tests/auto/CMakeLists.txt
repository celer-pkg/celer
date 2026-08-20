# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from auto.pro.
# exclude for CI build
add_subdirectory(quick3d_visual)
if(QT_BUILD_MINIMAL_STATIC_TESTS)
    return()
endif()
add_subdirectory(cmake)
if(NOT INTEGRITY)
    add_subdirectory(quick3d)
endif()
add_subdirectory(quick3d_particles)
add_subdirectory(utils)
add_subdirectory(tools)
if((android_app OR NOT ANDROID) AND (android_app OR NOT INTEGRITY) AND (NOT ANDROID OR NOT CMAKE_CROSSCOMPILING) AND (NOT ANDROID OR NOT WASM) AND (NOT CMAKE_CROSSCOMPILING OR NOT INTEGRITY) AND (NOT INTEGRITY OR NOT WASM))
    add_subdirectory(assetimport)
endif()
add_subdirectory(profiler)
