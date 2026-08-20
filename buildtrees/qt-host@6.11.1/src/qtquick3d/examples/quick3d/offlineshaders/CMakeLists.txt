# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(offlineshaders LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D)

qt_add_executable(offlineshaders
    main.cpp
)

set_target_properties(offlineshaders PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(offlineshaders PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
)

qt_add_qml_module(offlineshaders
    URI Example
    VERSION 1.0
    QML_FILES
        MaterialRed.qml
        MaterialRedExtended.qml
        main.qml
    RESOURCES
        maps/metallic/basecolor.jpg
    NO_RESOURCE_TARGET_PATH
)

#! [CMake material build function]
qt6_add_materials(offlineshaders "shaders"
    PREFIX
        /
    FILES
        MaterialRed.qml
        MaterialRedExtended.qml
        main.qml
)
#! [CMake material build function]

install(TARGETS offlineshaders
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET offlineshaders
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
