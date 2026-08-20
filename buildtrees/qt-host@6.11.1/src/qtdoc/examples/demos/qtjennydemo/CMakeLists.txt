# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
cmake_minimum_required(VERSION 3.22)

project(QtJenny VERSION 0.1 LANGUAGES CXX)

#! [0]
if (ANDROID)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set (gradlew_cmd "gradlew.bat")
    else()
         set (gradlew_cmd "./gradlew")
    endif()
    set (gradlew_arg "--rerun-tasks")
    set (gradlew_task "kaptReleaseKotlin")
    execute_process(COMMAND ${gradlew_cmd} ${gradlew_arg} ${gradlew_task}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/qtjenny_baseclass")
    execute_process(COMMAND ${gradlew_cmd} ${gradlew_arg} ${gradlew_task}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/qtjenny_general")
    execute_process(COMMAND ${gradlew_cmd} ${gradlew_arg} ${gradlew_task}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/qtjenny_callback")
else()
    message(FATAL_ERROR "Example only works on Android")
endif()
#! [0]

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick CorePrivate)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appqtjenny_consumer
    main.cpp
)

qt_add_qml_module(appqtjenny_consumer
    URI qtjenny_consumer
    QML_FILES
        Main.qml
    SOURCES
        backend.h
        backend.cpp
        volumeandbrightnessobserver.h
        volumeandbrightnessobserver.cpp
        contentobserver.h
        contentobserver.cpp
        nativedispatch.h
        nativedispatch.cpp
)

set_target_properties(appqtjenny_consumer PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

target_compile_definitions(appqtjenny_consumer
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appqtjenny_consumer
    PRIVATE Qt6::Quick Qt6::CorePrivate
)

include(GNUInstallDirs)
install(TARGETS appqtjenny_consumer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
