# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(manual_test_xr_gesture LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Quick Quick3D Quick3DXr)

qt6_policy(SET QTP0002 NEW)

qt_add_executable(manual_test_xr_gesture
    MANUAL_FINALIZATION
    main.cpp
    android/AndroidManifest.xml
)

set_target_properties(manual_test_xr_gesture PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(manual_test_xr_gesture PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
    Qt::Quick3DXr
)

qt_add_qml_module(manual_test_xr_gesture
    URI Example
    VERSION 1.0
    QML_FILES
        LeftHand.qml
        RightHand.qml
        main.qml
    NO_RESOURCE_TARGET_PATH
)

if(ANDROID)
    set_property(TARGET manual_test_xr_gesture APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()

if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    set_target_properties(manual_test_xr_gesture PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER io.qt.manual_test_xr_gesture
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/visionos/MacOSXBundleInfo.plist.in
    )
endif()

qt_finalize_executable(manual_test_xr_gesture)

install(TARGETS manual_test_xr_gesture
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET manual_test_xr_gesture
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
