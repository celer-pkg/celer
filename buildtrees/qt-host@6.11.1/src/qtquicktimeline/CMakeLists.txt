# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtquicktimeline.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtQuickTimeline # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Quick Timeline Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Quick)

if(NOT TARGET Qt::Quick)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Quick\" is not met.")
    return()
endif()

if(NOT QT_FEATURE_cborstreamreader OR NOT QT_FEATURE_cborstreamwriter)
    message(NOTICE "Skipping the build. QtQuickTimeline requires both 'cborstreamreader'"
        " and 'cborstreamwriter' features enabled in the Qt::Core module")
    return()
endif()

qt_build_repo()
