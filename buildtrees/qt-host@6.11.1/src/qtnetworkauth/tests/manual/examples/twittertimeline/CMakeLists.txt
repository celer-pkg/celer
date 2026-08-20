# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from twittertimeline.pro.

cmake_minimum_required(VERSION 3.16)
project(twittertimeline LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/oauth/twittertimeline")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Network)
find_package(Qt6 COMPONENTS NetworkAuth)

qt_add_executable(twittertimeline
    main.cpp
    twitter.cpp twitter.h
    twitterdialog.ui
    twittertimelinemodel.cpp twittertimelinemodel.h
)
set_target_properties(twittertimeline PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
)
target_link_libraries(twittertimeline PUBLIC
    Qt::Core
    Qt::Network
    Qt::NetworkAuth
    Qt::Widgets
)

install(TARGETS twittertimeline
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
