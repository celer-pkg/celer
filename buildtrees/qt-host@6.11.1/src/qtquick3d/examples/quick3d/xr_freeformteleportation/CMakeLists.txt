# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(xr_freeformteleportation LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Quick Quick3D Quick3DXr)

qt6_policy(SET QTP0002 NEW)

add_subdirectory("../xr_shared" "xr_shared")

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    main.cpp
    android/AndroidManifest.xml
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
    Qt::Quick3DXr
)

qt_add_qml_module(${PROJECT_NAME}
    URI Example
    VERSION 1.0
    QML_FILES
        main.qml
        Building.qml
    RESOURCES
        maps/Tape001_1K_NormalGL.jpg
        maps/Tape001_1K_Color.jpg
        maps/Tape001_1K_Roughness.jpg
        maps/Tiles107_1K_Color.jpg
        maps/Tiles107_1K_NormalGL.jpg
        maps/Tiles107_1K_Roughness.jpg
        maps/Tiles108_1K_Color.jpg
        meshes/building.mesh
    NO_RESOURCE_TARGET_PATH
)

if(ANDROID)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()

if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    set_target_properties(xr_freeformteleportation PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER io.qt.xr_freeformteleportation
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/visionos/MacOSXBundleInfo.plist.in
    )
endif()

qt_finalize_executable(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
