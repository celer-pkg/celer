# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qquickwindow.pro.

#####################################################################
## tst_qquickwindow Binary:
#####################################################################
cmake_minimum_required(VERSION 3.27)

qt_internal_add_benchmark(tst_qmlsortfilterproxymodel_bench
    SOURCES
        tst_qqmlsortfilterproxymodel.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARIES
        Qt::CorePrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::Test
)
