# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(custommaterial LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D)

qt_add_executable(custommaterial
    main.cpp
)

set_target_properties(custommaterial PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(custommaterial PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
)

qt_add_qml_module(custommaterial
    URI Example
    VERSION 1.0
    QML_FILES
        main.qml
    RESOURCES
        maps/OpenfootageNET_lowerAustria01-1024.hdr
        material_customlights.frag
        material_customspecular.frag
        material_distortion.vert
        material_metallic.frag
        material_simple.frag
        material_transparent.frag
        screen.frag
        weirdShape.mesh
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS custommaterial
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET custommaterial
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
