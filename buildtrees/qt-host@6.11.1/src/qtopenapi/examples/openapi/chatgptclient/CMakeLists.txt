# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.22)
project(appchat_example LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Quick OpenApi)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appchat_example
    main.cpp
    openaichatmanager.h
    openaichatmanager.cpp
)

qt_add_qml_module(appchat_example
    URI chat_example
    VERSION 1.0
    QML_FILES
        Main.qml
        EntryPage.qml
        ChatPage.qml
        UserPromtPage.qml
    RESOURCES
        resources/magic-hat.svg
        resources/head-plus.svg
        resources/head.svg
        resources/graduation-hat.svg
)

#! [0]
qt_add_library(openAIResponseApi)
qt_add_openapi_client(openAIResponseApi
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/response_api.yaml
)
#! [0]

target_link_libraries(appchat_example PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
    openAIResponseApi
)

install(TARGETS appchat_example
    BUNDLE  DESTINATION "${INSTALL_EXAMPLEDIR}"
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
