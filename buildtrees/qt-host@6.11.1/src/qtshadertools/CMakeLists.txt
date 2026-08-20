# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtshadertools.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtShaderTools
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt ShaderTools Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals) # special case

qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui)

if(INTEGRITY)
add_compile_definitions(__inline=inline)
endif()

if(NOT TARGET Qt::Gui)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\" is not met.")
    return()
endif()
if(WATCHOS)
    message(NOTICE "Skipping the build as the condition \"NOT WATCHOS\" is not met.")
    return()
endif()

# Ensure macros are available when configuring tests in-tree with the Qt build
include(tools/qsb/Qt6ShaderToolsMacros.cmake)

qt_build_repo()
