# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(volumeraycaster LANGUAGES CXX)
find_package(OpenMP)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Quick3D)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(volumeraycaster
    main.cpp
    volumetexturedata.cpp
    volumetexturedata.h
    lineboxgeometry.cpp
    lineboxgeometry.h
)

set_target_properties(volumeraycaster PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(volumeraycaster PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Quick3D
)

if(OpenMP_CXX_FOUND AND NOT ANDROID)
    target_link_libraries(volumeraycaster PUBLIC
        OpenMP::OpenMP_CXX
    )
endif()

qt_add_qml_module(volumeraycaster
    URI VolumetricExample
    VERSION 1.0
    QML_FILES
        Main.qml
        ArcballController.qml
        OriginGizmo.qml
        Spinner.qml
    RESOURCES
        images/circle.png
        images/colormap-coolwarm.png
        images/colormap-gist_rainbow.png
        images/colormap-gnuplot.png
        images/colormap-plasma.png
        images/colormap-rainbow.png
        images/colormap-viridis.png
        alpha_blending.frag
        alpha_blending.vert
        SpacingMap.mjs
    IMPORTS
        QtQuick3D
)

install(TARGETS volumeraycaster
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET volumeraycaster
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
