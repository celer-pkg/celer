# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(protobuf_sensors LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/protobuf/sensors")

find_package(Qt6 REQUIRED COMPONENTS Core Protobuf Widgets Network)

qt_standard_project_setup()

add_subdirectory(emulator)
add_subdirectory(client)

#! [0]
qt_add_protobuf(protobuf_sensors
    PROTO_FILES
        sensors.proto
        tlv.proto
)
#! [0]

install(TARGETS protobuf_sensors
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
