# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(DocumentViewer VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools
             OPTIONAL_COMPONENTS PrintSupport Pdf PdfWidgets Quick3D)

qt_standard_project_setup(REQUIRES 6.8
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES de
)

add_compile_definitions(QT_NO_CAST_FROM_ASCII)

add_subdirectory(plugins)
add_subdirectory(app)
