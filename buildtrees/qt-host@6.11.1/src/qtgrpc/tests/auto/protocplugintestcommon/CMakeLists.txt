# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_AUTOMOC TRUE)
qt_add_library(protocplugintestcommon STATIC protocplugintestcommon.h protocplugintestcommon.cpp)
qt_autogen_tools_initial_setup(protocplugintestcommon)

target_include_directories(protocplugintestcommon PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>")
target_link_libraries(protocplugintestcommon PUBLIC Qt6::Core Qt6::Test)
