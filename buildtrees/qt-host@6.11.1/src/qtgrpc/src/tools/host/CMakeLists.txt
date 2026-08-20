# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# Set up QT_HOST_PATH as an extra root path to look for the ProtobufTools and GrpcTools packages.
# If toolchain file already provides host paths in a predefined order, we shouldn't break it.
if(NOT "${QT_HOST_PATH_CMAKE_DIR}" IN_LIST CMAKE_PREFIX_PATH)
    list(PREPEND CMAKE_PREFIX_PATH "${QT_HOST_PATH_CMAKE_DIR}")
endif()
if(NOT "${QT_HOST_PATH}" IN_LIST CMAKE_FIND_ROOT_PATH)
    list(PREPEND CMAKE_FIND_ROOT_PATH "${QT_HOST_PATH}")
endif()
if(NOT "${QT_HOST_PATH}" IN_LIST CMAKE_PROGRAM_PATH)
    list(PREPEND CMAKE_PROGRAM_PATH "${QT_HOST_PATH}")
endif()

# This can't use the find_package(Qt6 COMPONENTS) signature, because Qt6Config uses NO_DEFAULT
# and won't look at the prepend extra find root paths.
find_package(Qt6ProtobufTools ${PROJECT_VERSION} CONFIG
    PATHS
        ${_qt_additional_packages_prefix_path}
        ${_qt_additional_packages_prefix_path_env}
        ${QT_HOST_PATH_CMAKE_DIR}
    NO_DEFAULT_PATH
)

find_package(Qt6GrpcTools ${PROJECT_VERSION} CONFIG
    PATHS
        ${_qt_additional_packages_prefix_path}
        ${_qt_additional_packages_prefix_path_env}
        ${QT_HOST_PATH_CMAKE_DIR}
    NO_DEFAULT_PATH
)

# If crosscompiling promote the WrapProtoc::WrapProtoc target to global, since
# it should be reused as is anyway.
if(TARGET WrapProtoc::WrapProtoc)
    __qt_internal_promote_target_to_global(WrapProtoc::WrapProtoc)
endif()

# Expose findings to the parent scope
set(Qt6ProtobufTools_TARGETS "${Qt6ProtobufTools_TARGETS}" PARENT_SCOPE)
set(Qt6GrpcTools_TARGETS "${Qt6GrpcTools_TARGETS}" PARENT_SCOPE)
