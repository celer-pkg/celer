# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.14)
set(appname "qtdice")
project(Dice LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D Quick3DPhysics Multimedia Sensors)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${appname} WIN32 MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(${appname} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Quick3D
    Qt6::Quick3DPhysics
    Qt6::Multimedia
    Qt6::Sensors
)

if(ANDROID)
    set_target_properties(${appname} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_PACKAGE_NAME "io.qt.dice"
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_VERSION_CODE 10
        QT_ANDROID_VERSION_NAME "1.0"
    )
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        qt_import_plugins(${appname}
            INCLUDE_BY_TYPE imageformats Qt::QSvgPlugin Qt::QJpegPlugin
            EXCLUDE_BY_TYPE qmltooling
            EXCLUDE_BY_TYPE iconengines
            EXCLUDE_BY_TYPE networkinformation
            EXCLUDE_BY_TYPE tls
            EXCLUDE_BY_TYPE platforminputcontexts
        )
    endif()
endif()

if(IOS)
    set(asset_catalog_path "ios/Assets.xcassets")
    target_sources(${appname} PRIVATE "${asset_catalog_path}")
    set_source_files_properties(${asset_catalog_path} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(${appname}
        PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
endif()

qt_add_qml_module(${appname}
    URI DiceModule
    QML_FILES
        Main.qml
        Dice_low.qml
        DiceSpawner.qml
        PhysicalDie.qml
        PhysicalTable.qml
        Scene.qml
        RoundTable.qml
        Carpet.qml
    RESOURCES
        #icons
        Menu_Icon.svg

        #maps
        maps/carpet_texture_baseColor.jpeg
        maps/M_Side_Table_Natural_Wenge_Wood_4K_baseColor_small.jpg
        maps/M_Side_Table_Natural_Wenge_Wood_4K_metallicRoughness_small.png
        maps/M_Side_Table_Natural_Wenge_Wood_4K_normal_small.png

        #meshes
        meshes/cube_001.mesh
        meshes/object_2.mesh
        meshes/object_3.mesh
        meshes/side_Table_Pine_LOD0_M_Side_Table_Natural_Wenge_Wood_4K_0.mesh

        #sounds
        sounds/onedice.wav
        sounds/rolling.wav
)


install(TARGETS ${appname}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ${appname}
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
