# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qcpainterbench LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick CanvasPainter)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(qcpainterbench
    WIN32
    MACOSX_BUNDLE
    main.cpp
)

qt_add_qml_module(qcpainterbench
    URI
        QCPainterBench
    SOURCES
        demoqcpainteritem.cpp demoqcpainteritem.h
        demoqcpainterrenderer.cpp demoqcpainterrenderer.h
        demoqpitem.cpp demoqpitem.h
    QML_FILES
        main.qml
        qml/Button.qml
        qml/FpsItem.qml
        qml/Switch.qml
        qml/SettingsView.qml
        qml/SliderSelector.qml
        qml/TabView.qml
        qml/DebugView.qml
    RESOURCE_PREFIX
        /
    RESOURCES
        qml/images/spinner.png
        qml/images/ic_settings_white_48dp.png
        qml/images/ic_cancel_white_48dp.png
        qml/images/ic_pause_white_48dp.png
        qml/images/ic_play_arrow_white_48dp.png
        qml/images/circle.png
        qml/fonts/Roboto-Regular.ttf
    NO_RESOURCE_TARGET_PATH
)

target_link_libraries(qcpainterbench PRIVATE
    Qt6::Gui
    Qt6::Quick
    Qt6::CanvasPainter
)

install(TARGETS qcpainterbench
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET qcpainterbench
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
