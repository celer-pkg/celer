# Copyright (C) 2025 The Qt Company Ltd.
# PDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(softwarekeyboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appsoftwarekeyboard
    main.cpp
)

qt_add_qml_module(appsoftwarekeyboard
    URI softwarekeyboard
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(appsoftwarekeyboard
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appsoftwarekeyboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
