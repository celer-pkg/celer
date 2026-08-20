# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## TinyCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(TinyCanBusPlugin
    OUTPUT_NAME qttinycanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        tinycan_symbols_p.h
        tinycanbackend.cpp tinycanbackend.h tinycanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)
