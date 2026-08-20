# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(chatserver LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core WebChannel WebSockets)

qt_add_executable(chatserver
    ../shared/websocketclientwrapper.cpp ../shared/websocketclientwrapper.h
    ../shared/websockettransport.cpp ../shared/websockettransport.h
    chatserver.cpp chatserver.h
    main.cpp
)

set_target_properties(chatserver PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(chatserver PUBLIC
    Qt::Core
    Qt::WebChannel
    Qt::WebSockets
)

install(TARGETS chatserver
    BUNDLE DESTINATION .
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
qt_generate_deploy_app_script(
    TARGET chatserver
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
