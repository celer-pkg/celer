# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## VectorCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VectorCanBusPlugin
    OUTPUT_NAME qtvectorcanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        vectorcan_symbols_p.h
        vectorcanbackend.cpp vectorcanbackend.h vectorcanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)
