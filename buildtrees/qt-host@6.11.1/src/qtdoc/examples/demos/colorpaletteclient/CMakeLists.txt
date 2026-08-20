# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(RESTfulAPIClient LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Network Gui Qml Quick)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(colorpaletteclient
    main.cpp
)

set_target_properties(colorpaletteclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

add_subdirectory(QtExampleStyle)

qt_add_qml_module(colorpaletteclient
    URI ColorPalette
    QML_FILES
        ColorPalette/Main.qml
        ColorPalette/ServerSelection.qml
        ColorPalette/ColorView.qml
        ColorPalette/ColorDialogEditor.qml
        ColorPalette/ColorDialogDelete.qml
        ColorPalette/UserMenu.qml
    SOURCES
        abstractresource.h
        basiclogin.h basiclogin.cpp
        restservice.h restservice.cpp
        paginatedresource.h paginatedresource.cpp
    RESOURCES
        icons/qt.png
        icons/testserver.png
        icons/userMask.svg
        icons/close.svg
        icons/delete.svg
        icons/dots.svg
        icons/edit.svg
        icons/login.svg
        icons/logout.svg
        icons/ok.svg
        icons/update.svg
        icons/user.svg
        icons/plus.svg
)

target_link_libraries(colorpaletteclient PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

install(TARGETS colorpaletteclient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET colorpaletteclient
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
