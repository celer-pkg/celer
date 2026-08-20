# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(ndefeditor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/nfc/ndefeditor")

find_package(Qt6 REQUIRED COMPONENTS Core Nfc Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(ndefeditor
    main.cpp
)

set(icon_files)
foreach(icon IN ITEMS add arrow_back file_download file_upload link text_snippet)
    foreach(scale IN ITEMS "" "@2" "@3" "@4")
        list(APPEND icon_files "icons/ndefeditor/20x20${scale}/${icon}.png")
    endforeach()
endforeach()

qt_add_resources(ndefeditor "theme" FILES
    ${icon_files}
    icons/ndefeditor/index.theme
)

qt_add_qml_module(ndefeditor
    URI NdefEditorModule
    VERSION 1.0
    QML_FILES
        Main.qml
        MainWindow.qml
        NdefRecordDelegate.qml
    SOURCES
        nfcmanager.h nfcmanager.cpp
        nfctarget.h nfctarget.cpp
        ndefmessagemodel.h ndefmessagemodel.cpp
)

set_target_properties(ndefeditor PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(IOS)
    set_target_properties(ndefeditor PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.cmake.plist"
    )
endif()

target_link_libraries(ndefeditor PRIVATE
    Qt6::Core
    Qt6::Nfc
    Qt6::Quick
)

install(TARGETS ndefeditor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
