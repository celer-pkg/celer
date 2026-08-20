# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## localsockettestserver Binary:
#####################################################################

qt_internal_add_executable(localsockettestserver
    SOURCES
        main.cpp
    LIBRARIES
        Qt::RemoteObjects
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)
