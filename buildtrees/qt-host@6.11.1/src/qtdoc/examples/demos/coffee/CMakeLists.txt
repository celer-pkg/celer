# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(CoffeeMachine LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)
qt_standard_project_setup(REQUIRES 6.6)

qt_add_executable(coffeemachine
    main.cpp
)

set_source_files_properties(Colors.qml Config.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(coffeemachine
    URI demos.coffee
    QML_FILES
        ApplicationFlow.qml
        ApplicationFlowForm.ui.qml
        ChoosingCoffeeForm.ui.qml
        ChoosingCoffee.qml
        CoffeeCard.qml
        CoffeeCardForm.ui.qml
        CoffeeConfig.qml
        Colors.qml
        Config.qml
        CustomButton.qml
        CustomButtonForm.ui.qml
        CustomSlider.qml
        CustomSliderForm.ui.qml
        CustomToolBar.qml
        CustomToolBarForm.ui.qml
        Cup.ui.qml
        Home.qml
        HomeForm.ui.qml
        Insert.qml
        InsertForm.ui.qml
        Main.qml
        Progress.qml
        ProgressForm.ui.qml
        Ready.qml
        ReadyForm.ui.qml
        Settings.qml
        SettingsForm.ui.qml
    RESOURCES
        qtquickcontrols2.conf
        images/Cups/card_cup_dark.svg
        images/Cups/card_cup_light.svg
        images/Cups/dark_cup.svgz
        images/Cups/home_dark.svg
        images/Cups/home_light.svg
        images/Cups/light_cup.svgz
        images/icons/check.svg
        images/icons/dark_mode_black_24dp.svg
        images/icons/ellipse_dark.svg
        images/icons/ellipse_light.svg
        images/icons/keyboard_backspace_black.svg
        images/icons/keyboard_backspace_black_left.svg
        images/icons/keyboard_backspace_black_right.svg
        images/icons/keyboard_backspace_white_left.svg
        images/icons/keyboard_backspace_white_right.svg
        images/icons/light_mode_black_24dp.svg
        images/icons/Polygon.svg
        images/icons/Qt-logo-white-transparent.svg
        images/Ingredients/espresso_coffee.svg
        images/Ingredients/Milk_foam.svg
        images/Ingredients/milk.svg
        images/Ingredients/sugar.svg
)
if(ANDROID)
    set_target_properties(coffeemachine PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_PACKAGE_NAME "io.qt.coffeemachine"
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_VERSION_CODE 20
        QT_ANDROID_VERSION_NAME "2.0"
    )
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        qt_import_plugins(coffeemachine
            INCLUDE_BY_TYPE imageformats Qt::QSvgPlugin
            EXCLUDE_BY_TYPE qmltooling
            EXCLUDE_BY_TYPE iconengines
            EXCLUDE_BY_TYPE networkinformation
            EXCLUDE_BY_TYPE tls
            EXCLUDE_BY_TYPE platforminputcontexts
        )
    endif()
endif()

set_target_properties(coffeemachine PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(IOS)
    set(asset_catalog_path "ios/Assets.xcassets")
    target_sources(coffeemachine PRIVATE "${asset_catalog_path}")
    set_source_files_properties(${asset_catalog_path} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(coffeemachine
        PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
endif()

target_link_libraries(coffeemachine PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

install(TARGETS coffeemachine
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET coffeemachine
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
