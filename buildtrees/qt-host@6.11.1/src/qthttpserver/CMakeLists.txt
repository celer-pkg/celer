# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qthttpserver.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtHttpServer
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt HTTP Server"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Concurrent WebSockets Gui)
if(NOT QT_FEATURE_mimetype)
    message(NOTICE "QHttpServer mimetype-dependency missing, skipping build")
    return()
endif()

qt_build_repo()
