# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(".cmake.conf")
project(QtActiveQt
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Active Qt Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

if (NOT WIN32)
    message(NOTICE "Condition \"WIN32\" is not met, enabling documentation build only.")
    # special case begin
    set(QT_NO_INTERNAL_COMPATIBILITY_FUNCTIONS TRUE)
    find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)
    qt_internal_project_setup()

    qt_build_repo_begin()
    add_subdirectory(src/activeqt/doc)
    qt_build_repo_end()
    # special case end
    return()
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Qml Quick Gui Widgets
    PrintSupport OpenGL OpenGLWidgets
)

if(NOT TARGET Qt::Gui OR NOT TARGET Qt::Widgets OR NOT TARGET Qt::PrintSupport)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Gui\", \"TARGET Qt::Widget\" "
            "or \"TARGET Qt::Printsupport\" is not met.")
    return()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/src/activeqt/container/Qt6AxContainerMacros.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/src/activeqt/control/Qt6AxServerMacros.cmake")

qt_build_repo()
