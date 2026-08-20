# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qtdoc.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtDoc # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Documentation and examples" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG
    OPTIONAL_COMPONENTS
        Gui
        Widgets
        PrintSupport
        Qml
        Quick
        Quick3D
        Quick3DXr
        Quick3DPhysics
        Test
        Sql
        Network
        QuickControls2
        QmlXmlListModel
        Graphs
        WebSockets
        Positioning
        Location
        Svg
        Sensors
        Multimedia
        Charts
        Pdf
        PdfWidgets
        TaskTree
        Linguist
)

find_package(Qt6 QUIET CONFIG OPTIONAL_COMPONENTS QuickStudioComponents)

qt_build_repo_begin()
qt_build_repo_impl_find_package_tests()

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILD_STANDALONE_EXAMPLES)
    add_subdirectory(doc)
    add_subdirectory(doc/src/cmake)
    add_subdirectory(doc/src/platformintegration)
    add_dependencies(QtDoc qtcmake)
    add_dependencies(QtDoc qtplatformintegration)

    # Dummy file that needs to be installed, to circumvent Coin skipping builds of standalone
    # tests due to not having anything to upload as an artifact.
    qt_configure_file(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qtdoc_dummy_file.txt" CONTENT " ")
    qt_install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qtdoc_dummy_file.txt" DESTINATION "${INSTALL_MKSPECSDIR}")
endif()

qt_build_repo_impl_src()
qt_build_repo_post_process()
qt_build_repo_impl_tests()
qt_build_repo_end()
qt_build_repo_impl_examples()
