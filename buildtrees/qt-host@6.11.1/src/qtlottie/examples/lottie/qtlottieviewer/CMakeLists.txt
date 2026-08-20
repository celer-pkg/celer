# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(qtlottieviewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(qtlottieviewerexample
    main.cpp
)

set(default_lottie_file "logo.json")
set_source_files_properties(${default_lottie_file} PROPERTIES
    QT_RESOURCE_ALIAS default_file.json
)

qt_add_qml_module(qtlottieviewerexample
    URI qtlottieviewer
    VERSION 1.0
    QML_FILES
        Main.qml
    RESOURCES
        ${default_lottie_file}
    RESOURCE_PREFIX
        /
)

target_link_libraries(qtlottieviewerexample PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

set(resource_files
    "qtquickcontrols2.conf"
)

qt_add_resources(qtlottieviewerexample "res"
    PREFIX
        "/"
    FILES
        ${resource_files}
)

install(TARGETS qtlottieviewerexample
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET qtlottieviewerexample
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
