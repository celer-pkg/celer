# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(fluidlauncher LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/embedded/fluidlauncher")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(fluidlauncher
    demoapplication.cpp demoapplication.h
    fluidlauncher.cpp fluidlauncher.h
    main.cpp
    pictureflow.cpp pictureflow.h
    slideshow.cpp slideshow.h
)

set_target_properties(fluidlauncher PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(fluidlauncher PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

# Resources:
set(fluidlauncher_resource_files
    "config.xml"
    "screenshots/anomaly_s60.png"
    "screenshots/concentriccircles.png"
    "screenshots/context2d_s60.png"
    "screenshots/deform.png"
    "screenshots/desktopservices_s60.png"
    "screenshots/digiflip.png"
    "screenshots/elasticnodes.png"
    "screenshots/embeddedsvgviewer.png"
    "screenshots/embeddedsvgviewer_s60.png"
    "screenshots/flickable.png"
    "screenshots/flightinfo_s60.png"
    "screenshots/fridgemagnets_s60.png"
    "screenshots/ftp_s60.png"
    "screenshots/lightmaps.png"
    "screenshots/mediaplayer.png"
    "screenshots/pathstroke.png"
    "screenshots/qmlcalculator.png"
    "screenshots/qmlclocks.png"
    "screenshots/qmldialcontrol.png"
    "screenshots/qmleasing.png"
    "screenshots/qmlflickr.jpg"
    "screenshots/qmlphotoviewer.jpg"
    "screenshots/qmltwitter.jpg"
    "screenshots/raycasting.png"
    "screenshots/saxbookmarks_s60.png"
    "screenshots/softkeys_s60.png"
    "screenshots/spectrum.png"
    "screenshots/styledemo.png"
    "screenshots/styledemo_s60.png"
    "screenshots/wiggly.png"
    "screenshots/wiggly_s60.png"
    "slides/demo_1.png"
    "slides/demo_2.png"
    "slides/demo_3.png"
    "slides/demo_4.png"
    "slides/demo_5.png"
    "slides/demo_6.png"
)

qt6_add_resources(fluidlauncher "fluidlauncher"
    PREFIX
        "/fluidlauncher"
    FILES
        ${fluidlauncher_resource_files}
)

install(TARGETS fluidlauncher
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
