# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## SocketCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(SocketCanBusPlugin
    OUTPUT_NAME qtsocketcanbus
    PLUGIN_TYPE canbus
    SOURCES
        libsocketcan.cpp libsocketcan.h
        main.cpp
        socketcanbackend.cpp socketcanbackend.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)
