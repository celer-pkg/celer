# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtSvg
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt SVG Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui Widgets Xml Network)

# Needed for examples to configure properly when not using ExternalProject and when
# Qt is configured with a bundled zlib.
# Otherwise Qt6SvgDependencies will claim Qt6ZlibPrivate_FOUND is FALSE because we
# set QT_NO_CREATE_TARGETS to TRUE when building examples in-tree.
if(NOT QT_FEATURE_system_zlib)
    find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS ZlibPrivate)
endif()

qt_build_repo()
