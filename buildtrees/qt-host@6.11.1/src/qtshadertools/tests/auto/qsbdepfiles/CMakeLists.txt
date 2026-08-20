# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qsbdepfiles LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qsbdepfiles
    SOURCES
    tst_qsbdepfiles.cpp
)

qt_add_shaders(tst_qsbdepfiles
    "shaders"
    PRECOMPILE
    OPTIMIZED
    PREFIX
        "/"
    FILES
        "shaders/tst.frag"
        "shaders/tstinclude.frag"
        "shaders/tstincluderelative.frag"
        "shaders/tstincludewhitespaces.frag"
        "shaders/tstincludecomment.frag"
)

set(data_base_dir "${CMAKE_CURRENT_BINARY_DIR}/data")

configure_file("data/tst.frag.qsb.d.in"
    "${data_base_dir}/tst.frag.qsb.d" NEWLINE_STYLE LF)
configure_file("data/tstinclude.frag.qsb.d.in"
    "${data_base_dir}/tstinclude.frag.qsb.d" NEWLINE_STYLE LF)
configure_file("data/tstincluderelative.frag.qsb.d.in"
    "${data_base_dir}/tstincluderelative.frag.qsb.d" NEWLINE_STYLE LF)
configure_file("data/tstincludewhitespaces.frag.qsb.d.in"
    "${data_base_dir}/tstincludewhitespaces.frag.qsb.d" NEWLINE_STYLE LF)
configure_file("data/tstincludecomment.frag.qsb.d.in"
    "${data_base_dir}/tstincludecomment.frag.qsb.d" NEWLINE_STYLE LF)

qt_add_resources(tst_qsbdepfiles "tst_qsbdepfiles_data"
    PREFIX "/data"
    BASE "${data_base_dir}"
    FILES
        "${data_base_dir}/tst.frag.qsb.d"
        "${data_base_dir}/tstinclude.frag.qsb.d"
        "${data_base_dir}/tstincluderelative.frag.qsb.d"
        "${data_base_dir}/tstincludewhitespaces.frag.qsb.d"
        "${data_base_dir}/tstincludecomment.frag.qsb.d"
)

