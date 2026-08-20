# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(ftpclient LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/ftpclient")

find_package(Qt6 REQUIRED COMPONENTS Core Network Scxml)

qt_standard_project_setup()

qt_add_executable(ftpclient
    ftpcontrolchannel.cpp ftpcontrolchannel.h
    ftpdatachannel.cpp ftpdatachannel.h
    main.cpp
)

set_target_properties(ftpclient PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(ftpclient PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Scxml
)

# Statecharts:
qt6_add_statecharts(ftpclient
    simpleftp.scxml
)

install(TARGETS ftpclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
