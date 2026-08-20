# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## scrambledSignals Binary:
#####################################################################

qt_internal_add_executable(scrambledSignals
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    SOURCES
        ../mismatch.cpp
    INCLUDE_DIRECTORIES
        ..
    LIBRARIES
        Qt::RemoteObjects
        Qt::Test
)
qt6_add_repc_replicas(scrambledSignals
    mismatch.rep
)
