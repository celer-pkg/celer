# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtCanvasPainter
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt CanvasPainter Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui Widgets Quick)

# Set up QT_HOST_PATH as an extra root path to look for the ShaderToolsTools package
# when cross-compiling.
if(NOT "${QT_HOST_PATH}" STREQUAL "")
     set(_qt_backup_qtdeclarative_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
     set(_qt_backup_qtdeclarative_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
     list(PREPEND CMAKE_PREFIX_PATH "${QT_HOST_PATH_CMAKE_DIR}")
     list(PREPEND CMAKE_FIND_ROOT_PATH "${QT_HOST_PATH}")
endif()

# This can't use the find_package(Qt6 COMPONENTS) signature, because Qt6Config uses NO_DEFAULT and
# won't look at the prepend extra find root paths.
find_package(Qt6ShaderToolsTools ${PROJECT_VERSION} QUIET CONFIG
    PATHS
            ${_qt_additional_packages_prefix_path}
            ${_qt_additional_packages_prefix_path_env}
)

if(NOT "${QT_HOST_PATH}" STREQUAL "")
     set(CMAKE_PREFIX_PATH ${_qt_backup_qtdeclarative_CMAKE_PREFIX_PATH})
     set(CMAKE_FIND_ROOT_PATH ${_qt_backup_qtdeclarative_CMAKE_FIND_ROOT_PATH})
endif()

# Optionally look for the target ShaderTools package to avoid warnings
# when configuring the project to build examples in-tree in a prefix Qt build.
# See QTBUG-96358 for details.
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS ShaderTools)

include(tools/qcshadergen/Qt6CanvasPainterMacros.cmake)

qt_build_repo()
