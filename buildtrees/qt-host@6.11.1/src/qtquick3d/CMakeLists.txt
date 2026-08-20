# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtquick3d.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtQuick3D
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Quick3D Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Concurrent Network Quick
    Widgets PacketProtocolPrivate QuickTestUtilsPrivate QmlModelsPrivate)

include(src/quick3d/Qt6Quick3DMacros.cmake)

if(WATCHOS)
    message(NOTICE "Skipping the build as the condition \"NOT WATCHOS\" is not met.")
    return()
endif()
if(NOT TARGET Qt::Quick)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Quick\" is not met.")
    return()
endif()

# Set up QT_HOST_PATH as an extra root path to look for the ShaderToolsTools package
# when cross-compiling.
if(NOT "${QT_HOST_PATH}" STREQUAL "")
     set(_qt_backup_qtshadertools_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
     set(_qt_backup_qtshadertools_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
     list(PREPEND CMAKE_PREFIX_PATH "${QT_HOST_PATH_CMAKE_DIR}")
     list(PREPEND CMAKE_FIND_ROOT_PATH "${QT_HOST_PATH}")
endif()

# This can't use the find_package(Qt6 COMPONENTS) signature, because Qt6Config uses NO_DEFAULT and
# won't look at the prepend extra find root paths.
find_package(Qt6ShaderToolsTools ${PROJECT_VERSION} QUIET CONFIG REQUIRED
    PATHS
            ${_qt_additional_packages_prefix_path}
            ${_qt_additional_packages_prefix_path_env}
)

if(NOT "${QT_HOST_PATH}" STREQUAL "")
     set(CMAKE_PREFIX_PATH ${_qt_backup_qtshadertools_CMAKE_PREFIX_PATH})
     set(CMAKE_FIND_ROOT_PATH ${_qt_backup_qtshadertools_CMAKE_FIND_ROOT_PATH})
endif()

# Look for the target ShaderTools package to have access to the ShaderTools Qt module library.
# This is optional. When not present, runtime shader processing will not be available, and
# only materials pre-generated with the shadergen tool will be functional.
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS ShaderTools)

# Look for the target QuickTimeline package to have access to the QuickTimeline Qt module library.
# This is optional. When not present, runtime assets' timeline animation will not work.
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS QuickTimeline)

qt_build_repo()
