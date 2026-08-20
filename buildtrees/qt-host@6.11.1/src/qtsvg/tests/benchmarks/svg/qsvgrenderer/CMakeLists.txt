# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_bench_qsvgrenderer Binary:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_bench_qsvgrenderer LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_benchmark(tst_bench_qsvgrenderer
    SOURCES
        tst_qsvgrenderer.cpp
    LIBRARIES
        Qt::Gui
        Qt::Test
        Qt::Svg
)

qt_internal_add_resource(tst_bench_qsvgrenderer "tst_bench_qsvgrenderer_dat"
    PREFIX
        "/"
    FILES
        "./data/tiger.svg"
        "./data/filter.svg"
        "./data/mask.svg"
        "./data/opacity.svg"
        "./data/text.svg"
)
