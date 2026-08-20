# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## signatureServer Binary:
#####################################################################

qt_internal_add_executable(signatureServer
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/rep_server_source.h
        main.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        Qt::RemoteObjects
        Qt::Test
)
qt6_add_repc_sources(signatureServer
    ../server.rep
)
