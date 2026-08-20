# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(particles3d LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D)

qt_add_executable(particles3d
    main.cpp
)

set_target_properties(particles3d PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(particles3d PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
)

set_source_files_properties(AppSettings.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(particles3d
    URI Example
    VERSION 1.0
    QML_FILES
        AlignedParticles.qml
        AnimatedSprite.qml
        AttractorShapes.qml
        ColorfulParticles.qml
        CustomCheckBox.qml
        CustomLabel.qml
        CustomSlider.qml
        CustomSelectionBox.qml
        DynamicBursts.qml
        EmitAndBurst.qml
        EmitterCustomShapes.qml
        EmitterShapes.qml
        FadingInOut.qml
        Fire.qml
        HeartTrail.qml
        Lights.qml
        LoggingView.qml
        ModelBlendParticles.qml
        OceanSpider.qml
        QtLogoAnimation.qml
        SettingsView.qml
        Snowing.qml
        Sorting.qml
        Speedometer.qml
        StartupView.qml
        SystemPlayPause.qml
        TrailEmitterBurst.qml
        ModelShape.qml
        main.qml
        LineParticles.qml
        AppSettings.qml
        SceneShape.qml
    RESOURCES
        images/arrow_icon.png
        images/bear_black.png
        images/colorTable.png
        images/color_table2.png
        images/color_table3.png
        images/color_table4.png
        images/color_table5.png
        images/dot.png
        images/dust.png
        images/explosion_01_strip13.png
        images/icon_interval.png
        images/icon_logging.png
        images/icon_pause.png
        images/icon_play.png
        images/icon_settings.png
        images/leather_n.png
        images/qt_logo.png
        images/qt_logo2.png
        images/qt_logo2_n.png
        images/smoke.png
        images/smoke_sprite.png
        images/snowflake.png
        images/speedometer_labels.png
        images/sphere.png
        images/sprite_09.png
        images/star.png
        images/star2.png
        images/star3.png
        meshes/meter_background.mesh
        meshes/meter_edge.mesh
        meshes/oldqtlogo.mesh
        meshes/suzanne.mesh
        data/qt_logo_in_4096.cbor
        data/qt_logo_out_4096.cbor
        data/heart_4096.cbor
    NO_RESOURCE_TARGET_PATH
)

install(TARGETS particles3d
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET particles3d
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
