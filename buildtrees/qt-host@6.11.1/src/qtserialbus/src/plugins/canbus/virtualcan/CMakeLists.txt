# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## VirtualCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VirtualCanBusPlugin
    OUTPUT_NAME qtvirtualcanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        virtualcanbackend.cpp virtualcanbackend.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)
