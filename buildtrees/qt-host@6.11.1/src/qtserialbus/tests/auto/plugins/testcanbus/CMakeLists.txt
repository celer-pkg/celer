# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## TestCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(TestCanBusPlugin
    OUTPUT_NAME qttestcanbus
    PLUGIN_TYPE canbus
    DEFAULT_IF FALSE
    STATIC
    SOURCES
        testcanbackend.cpp testcanbackend.h
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)
