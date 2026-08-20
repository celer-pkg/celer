# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(WindowEmbeddingExample LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(windowembedding
    WIN32
    MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(windowembedding PRIVATE
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
)

qt_add_qml_module(windowembedding URI windowembedding
    QML_FILES "Main.qml"
    OUTPUT_DIRECTORY windowembedding_module
)

if(APPLE)
    enable_language(OBJCXX)
    set_source_files_properties(main.cpp PROPERTIES LANGUAGE OBJCXX)
elseif(WIN32)
    target_link_libraries(windowembedding PRIVATE comctl32)
elseif(QT_FEATURE_xcb)
    find_package(GTK3)
    if(GTK3_FOUND)
        if(NOT QT_FEATURE_glib)
            message(FATAL_ERROR "GTK integration requires that Qt is built with glib support")
        endif()
        target_link_libraries(windowembedding PRIVATE PkgConfig::GTK3)
        target_compile_definitions(windowembedding PRIVATE QT_NO_SIGNALS_SLOTS_KEYWORDS)
    endif()
endif()

install(TARGETS windowembedding
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET windowembedding
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
