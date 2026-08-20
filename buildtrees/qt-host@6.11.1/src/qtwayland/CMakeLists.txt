# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtwayland.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtWayland # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Wayland Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    BuildInternals
)
qt_internal_project_setup()

if(NOT MACOS AND NOT QNX AND (ANDROID OR NOT LINUX))
    message(NOTICE "Skipping the build as the condition \"LINUX OR MACOS OR QNX\" is not met.")
    return()
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
    Core
)

find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS
    DBus
    Gui
    OpenGL
    Quick
    Svg
)

# Set up QT_HOST_PATH as an extra root path to look for the WaylandTools package
# when cross-compiling.
if(NOT "${QT_HOST_PATH}" STREQUAL "")
     set(_qt_backup_qtwayland_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
     set(_qt_backup_qtwayland_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
     list(PREPEND CMAKE_PREFIX_PATH "${QT_HOST_PATH_CMAKE_DIR}")
     list(PREPEND CMAKE_FIND_ROOT_PATH "${QT_HOST_PATH}")
endif()

# This can't use the find_package(Qt6 COMPONENTS) signature, because Qt6Config uses NO_DEFAULT and
# won't look at the prepend extra find root paths.
find_package(Qt6WaylandScannerTools ${PROJECT_VERSION} QUIET CONFIG
    PATHS
            ${_qt_additional_packages_prefix_path}
            ${_qt_additional_packages_prefix_path_env}
)

if(NOT "${QT_HOST_PATH}" STREQUAL "")
     set(CMAKE_PREFIX_PATH ${_qt_backup_qtwayland_CMAKE_PREFIX_PATH})
     set(CMAKE_FIND_ROOT_PATH ${_qt_backup_qtwayland_CMAKE_FIND_ROOT_PATH})
endif()

find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS WaylandClient)

# special case begin
# Moved above.
#if(NOT MACOS AND NOT QNX AND (ANDROID OR NOT LINUX))
#    message(NOTICE "Skipping the build as the condition \"LINUX OR MACOS OR QNX\" is not met.")
#    return()
#endif()
# special case end
if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
qt_build_repo()
