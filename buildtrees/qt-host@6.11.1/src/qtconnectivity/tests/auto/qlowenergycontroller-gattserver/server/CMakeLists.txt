# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## server Binary:
#####################################################################

qt_internal_add_executable(qlecontroller-server
    GUI
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    SOURCES
        qlowenergycontroller-gattserver.cpp
    LIBRARIES
        Qt::Bluetooth
)
