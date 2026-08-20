# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_qremoteobjects_module)

find_package(Qt6RemoteObjects REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(MAIN_SRCS main.cpp)
add_executable(mainapp ${MAIN_SRCS})
qt6_add_repc_replicas(mainapp ../../integration/pod.rep)

target_link_libraries(mainapp Qt::RemoteObjects)
