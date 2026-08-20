# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(oit3d LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D Quick3DParticles)

qt_add_executable(oit3d
    main.cpp
)

set_target_properties(oit3d PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(oit3d PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
    Qt::Quick3DParticles
)

set_source_files_properties(AppSettings.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(oit3d
    URI Example
    VERSION 1.0
    QML_FILES
        CustomCheckBox.qml
        CustomLabel.qml
        CustomSlider.qml
        CustomSelectionBox.qml
        SettingsView.qml
        StartupView.qml
        main.qml
        AppSettings.qml
        BlendValueTest.qml
        Particles.qml
        CustomInfoView.qml
        Instancing.qml
        FireStone.qml
        FireStick.qml
        FireParticles.qml
        RandomEmitter.qml
    RESOURCES
        images/arrow_icon.png
        images/icon_settings.png
        images/qt_logo.png
        images/qt_logo2.png
        images/qt_logo2_n.png
        images/tilepattern.png
        images/pillar_normal.png
        images/firestone.png
        images/stick_charred.png
        images/stick_heat3.png
        images/color_table3.png
        images/flame.png
        meshes/asteroid.mesh
        meshes/block.mesh
        meshes/stick3.mesh
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS oit3d
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET oit3d
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
