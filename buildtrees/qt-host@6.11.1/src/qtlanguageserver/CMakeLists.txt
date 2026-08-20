# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16.0)

include(.cmake.conf)
project(QtLanguageServer
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Language Server"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Network Concurrent Test)

if(ANDROID)
    # Android tests needs to link against Qt6::Gui
    find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui)
endif()

qt_build_repo()
