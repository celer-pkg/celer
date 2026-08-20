# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(imagescaling LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/tasktree/imagescaling")

find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Gui Network TaskTree Widgets)

qt_standard_project_setup()

qt_add_executable(task_tree_imagescaling
    downloaddialog.cpp
    downloaddialog.h
    downloaddialog.ui
    imagescaling.cpp
    imagescaling.h
    main.cpp
)

set_target_properties(task_tree_imagescaling PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(task_tree_imagescaling PUBLIC
    Qt::Concurrent
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::TaskTree
    Qt::Widgets
)

install(TARGETS task_tree_imagescaling
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
