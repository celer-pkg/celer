# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.19)

option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

project(CarConfigurator VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS
             Core
             Gui
             Qml
             QmlAssetDownloaderPrivate
             Quick
             Quick3D
             Quick3DParticles
             Svg
             TaskTree
             Network
             Concurrent
             OPTIONAL_COMPONENTS
             QuickStudioComponents)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(carconfigurator
    src/main.cpp
    src/app_environment.h
)

qt_add_resources(carconfigurator "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

target_link_libraries(carconfigurator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Quick3D
    Qt6::Qml
    Qt6::Svg
)

include(buildsetup.cmake)
include(qmlmodules.cmake)

if (NOT TARGET Qt6::QuickStudioComponents)
    include(qmlcomponents.cmake)
endif()

if (ANDROID)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
        )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)

    add_android_openssl_libraries(carconfigurator)
endif()

if(APPLE)
    set_target_properties(carconfigurator PROPERTIES
        OUTPUT_NAME "Car Configurator"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Car Configurator"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.qt.examples.carconfigurator"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

install(TARGETS carconfigurator
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET carconfigurator
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
