# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

qt_add_executable(protobuf_sensor_emulator
    main.cpp
    sensoremulator.h sensoremulator.cpp
    emulatorconsole.h emulatorconsole.cpp
    emulatorconsole.ui
)

target_link_libraries(protobuf_sensor_emulator PRIVATE
    Qt6::Core
    Qt6::Protobuf
    Qt6::Widgets
    Qt6::Network
    protobuf_sensors
)

install(TARGETS protobuf_sensor_emulator
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
