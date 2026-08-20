# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtOpenAPI
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt6 OpenAPI Client generator and bindings."
    HOMEPAGE_URL "https://qt.io"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS
    Network Gui Widgets Quick
    QmlNetwork QuickControls2
    TestInternalsPrivate
)

if(NOT TARGET Qt::Network)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Network\" is not met.")
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/src/openapi/Qt6OpenApiToolsGeneratorMacros.cmake")

qt_build_repo()
