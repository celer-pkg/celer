# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

get_filename_component(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)

qt_internal_add_test(tst_qcansignaldescription
    SOURCES
        tst_qcansignaldescription.cpp
    INCLUDE_DIRECTORIES
        ${SHARED_DIR}
    LIBRARIES
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
)
