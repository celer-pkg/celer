# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(tst_typelibs)

find_package(Qt6 COMPONENTS AxContainer Gui Widgets CONFIG REQUIRED)

add_executable(tst_typelibs main.cpp)
target_link_libraries(tst_typelibs PRIVATE Qt6::AxContainer Qt6::Gui Qt6::Widgets)
target_include_directories(tst_typelibs PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

qt6_target_typelibs(tst_typelibs LIBRARIES "ieframe.dll"
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/typelibs"
)
