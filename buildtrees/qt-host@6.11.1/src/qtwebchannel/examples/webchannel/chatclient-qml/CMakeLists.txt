# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qmlchatclient LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick WebChannel Widgets)

qt_standard_project_setup()

qt_add_executable(qmlchatclient
    main.cpp
)

set_target_properties(qmlchatclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(qmlchatclient PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Widgets
)

# Resources:
set(data_resource_files
    "qmlchatclient.qml"
    "LoginForm.ui.qml"
    "MainForm.ui.qml"
    "${QT_QWEBCHANNEL_JS_PATH}"
)

set_source_files_properties("${QT_QWEBCHANNEL_JS_PATH}" PROPERTIES
    QT_RESOURCE_ALIAS "qwebchannel.js"
)

qt6_add_resources(qmlchatclient "data"
    PREFIX
        "/"
    FILES
        ${data_resource_files}
)

install(TARGETS qmlchatclient
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
qt_generate_deploy_qml_app_script(
    TARGET qmlchatclient
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
