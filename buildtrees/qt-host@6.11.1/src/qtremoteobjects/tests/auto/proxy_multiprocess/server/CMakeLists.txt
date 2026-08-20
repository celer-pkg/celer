# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## proxy_multiprocess_server Binary:
#####################################################################

qt_internal_add_executable(proxy_multiprocess_server
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/rep_subclass_source.h
        ../namespace.h
        ../shared.h
        main.cpp
        mytestserver.cpp mytestserver.h
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        Qt::RemoteObjects
        Qt::Test
)
qt6_add_repc_sources(proxy_multiprocess_server
    ../subclass.rep
)
