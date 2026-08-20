# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qt5compat.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(Qt5Compat # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt 5 Compatibility Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Network Xml Gui Widgets Quick)

# Look for the target ShaderTools package to have access to the ShaderTools Qt module library.
# This is optional. When not present, runtime shader processing will not be available, and
# only a certain set of effects will be available
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS ShaderTools)

qt_build_repo()
