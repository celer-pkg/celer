# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtQuick3DPhysics
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Quick3D Physics Extensions"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# Make sure we only use latest private CMake API, aka no compatibility wrappers.
set(QT_NO_INTERNAL_COMPATIBILITY_FUNCTIONS TRUE)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Gui Qml Quick Quick3D QUIET)
if(QT_BUILD_TESTS OR QT_BUILD_STANDALONE_TESTS)
    find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS QuickTest)
endif()

if(INTEGRITY OR QNX OR VXWORKS OR WATCHOS)
    message(NOTICE "Skipping the build as the condition \"NOT INTEGRITY AND NOT QNX AND NOT VXWORKS AND NOT WATCHOS\" is not met.")
    return()
endif()
if(NOT TARGET Qt::Quick3D)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Quick3D\" is not met.")
    return()
endif()

qt_build_repo()
