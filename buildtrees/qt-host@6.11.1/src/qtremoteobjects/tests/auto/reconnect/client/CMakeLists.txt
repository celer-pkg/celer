# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_executable(qtro_reconnect_client
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    SOURCES main.cpp
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES Qt::RemoteObjects Qt::Test
)
