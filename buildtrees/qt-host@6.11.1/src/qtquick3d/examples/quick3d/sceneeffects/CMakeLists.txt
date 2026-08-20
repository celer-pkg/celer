# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(sceneeffects LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D)

set(CMAKE_AUTORCC ON)

qt_add_executable(sceneeffects
    assets.qrc
    luts.qrc
    main.cpp
)

set_target_properties(sceneeffects PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(sceneeffects PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
)

qt_add_qml_module(sceneeffects
    URI SceneEffectExample
    VERSION 1.0
    QML_FILES
        ColorPicker.qml
        SectionLayout.qml
        SettingsPage.qml
        main.qml
    NO_RESOURCE_TARGET_PATH
)


qt_add_shaders(sceneeffects "baked-shaders"
    PREFIX
        "/"
    FILES
        "shaders/huesaturation.frag"
)

install(TARGETS sceneeffects
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET sceneeffects
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
