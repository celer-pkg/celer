# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(task_tree_demo LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/tasktree/demo")

find_package(Qt6 REQUIRED COMPONENTS Core Gui TaskTree Widgets)

qt_standard_project_setup()

qt_add_executable(task_tree_demo
    main.cpp
    progressindicator.cpp
    progressindicator.h
    taskwidget.cpp
    taskwidget.h
)

set_target_properties(task_tree_demo PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(task_tree_demo PUBLIC
    Qt::Core
    Qt::Gui
    Qt::TaskTree
    Qt::Widgets
)

# Resources:
set(demo_resource_files
    "images/progressindicator.png"
)

qt_add_resources(task_tree_demo "demo"
    PREFIX
        "/"
    FILES
        ${demo_resource_files}
)

install(TARGETS task_tree_demo
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
