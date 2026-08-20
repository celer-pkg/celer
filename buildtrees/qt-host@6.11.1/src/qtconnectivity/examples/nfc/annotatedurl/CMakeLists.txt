# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(annotatedurl LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/nfc/annotatedurl")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Nfc Widgets)

qt_standard_project_setup()

qt_add_executable(annotatedurl
    MANUAL_FINALIZATION
    annotatedurl.cpp annotatedurl.h
    main.cpp
    mainwindow.cpp mainwindow.h
)

set_target_properties(annotatedurl PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(IOS)
    set_target_properties(annotatedurl PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
endif()

target_link_libraries(annotatedurl PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Nfc
    Qt6::Widgets
)

if(ANDROID)
    set_property(TARGET annotatedurl
        APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/android
    )
endif()

qt_finalize_target(annotatedurl)

install(TARGETS annotatedurl
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
