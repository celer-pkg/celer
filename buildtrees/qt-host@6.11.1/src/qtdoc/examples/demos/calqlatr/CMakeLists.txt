# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(Calqlatr LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(calqlatr WIN32 MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(calqlatr PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

qt_add_qml_module(calqlatr
    URI demos.calqlatr
    QML_FILES
        "Main.qml"
        "content/ApplicationState.qml"
        "content/Display.qml"
        "content/NumberPad.qml"
        "content/CalculatorButton.qml"
        "content/BackspaceButton.qml"
        "content/calculator.js"
    RESOURCES
        "content/images/backspace.svg"
        "content/images/backspace_fill.svg"
)

if(ANDROID)
    set_target_properties(calqlatr PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_PACKAGE_NAME "io.qt.calqlatr"
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_VERSION_CODE 10
        QT_ANDROID_VERSION_NAME "1.0"
    )
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        qt_import_plugins(calqlatr
            INCLUDE_BY_TYPE imageformats Qt::QSvgPlugin
            EXCLUDE_BY_TYPE qmltooling
            EXCLUDE_BY_TYPE iconengines
            EXCLUDE_BY_TYPE networkinformation
            EXCLUDE_BY_TYPE tls
            EXCLUDE_BY_TYPE platforminputcontexts
        )
    endif()
endif()

if(APPLE)
    set_target_properties(calqlatr PROPERTIES
        OUTPUT_NAME "Calqlatr"
        MACOSX_BUNDLE_BUNDLE_NAME "Calqlatr"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.calqlatr"
    )
    if(IOS)
        set(asset_catalog_path "ios/Assets.xcassets")
        target_sources(calqlatr PRIVATE "${asset_catalog_path}")
        set_source_files_properties(${asset_catalog_path} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_target_properties(calqlatr
            PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
    endif()
endif()

if(WIN32)
    target_sources(calqlatr PRIVATE windows/calqlatr.rc)
endif()

install(TARGETS calqlatr
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET calqlatr
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
