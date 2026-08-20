# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(lodhelper LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D)

qt_add_executable(lodhelper
    main.cpp
)

set_target_properties(lodhelper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(lodhelper PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
)

qt_add_qml_module(lodhelper
    URI Example
    VERSION 1.0
    QML_FILES
        main.qml
    RESOURCES
        meshes/marble_bust_01_LOD_0.mesh
        meshes/marble_bust_01_LOD_1.mesh
        meshes/marble_bust_01_LOD_2.mesh
        meshes/marble_bust_01_LOD_3.mesh
        maps/baseColor.png
        maps/normal.png
        maps/occlusionRoughnessMetallic.png
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS lodhelper
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET lodhelper
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
