# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(RSSNews LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml QmlXmlListModel Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(rssnews WIN32 MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(rssnews PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QmlXmlListModel
)

qt_add_qml_module(rssnews
    URI RssNewsModule
    QML_FILES
        content/BusyIndicator.qml
        content/CategoryDelegate.qml
        content/NewsDelegate.qml
        content/RssFeeds.qml
        content/ScrollBar.qml
        Main.qml
    RESOURCES
        content/images/Asia.jpg
        content/images/Business.jpg
        content/images/Entertainment.jpg
        content/images/Europe.jpg
        content/images/Health.jpg
        content/images/Politics.jpg
        content/images/Science.jpg
        content/images/Sports.jpg
        content/images/Technology.jpg
        content/images/TopStories.jpg
        content/images/USNational.jpg
        content/images/World.jpg
        content/images/btn_close.png
        content/images/busy.png
        content/images/scrollbar.png
)

install(TARGETS rssnews
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET rssnews
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
