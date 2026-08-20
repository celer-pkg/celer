# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qsvgplugin Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qsvgplugin LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qsvgplugin
    SOURCES
        tst_qsvgplugin.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::SvgPrivate
)

# Resources:
set(resources_resource_files
    "imageInclude.svg"
    "imageIncludeA.svg"
    "checkers.png"
    "square.svg"
    "square_size.svg"
    "square_size_viewbox.svg"
    "square_viewbox.svg"
    "tall.svg"
    "tall_size.svg"
    "tall_size_viewbox.svg"
    "tall_viewbox.svg"
    "wide.svg"
    "wide_size.svg"
    "wide_size_viewbox.svg"
    "wide_viewbox.svg"
    "simple_Utf8.svg"
    "simple_Utf8.svgz"
    "simple_Utf16LE.svg"
    "simple_Utf16BE.svg"
    "simple_Utf32LE.svg"
    "simple_Utf32BE.svg"
    "simple_Utf32BE.svg.gz"
    "invalid_xml.svg"
    "xml_not_svg.svg"
    "invalid_then_valid.svg"
)

qt_internal_add_resource(tst_qsvgplugin "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)
