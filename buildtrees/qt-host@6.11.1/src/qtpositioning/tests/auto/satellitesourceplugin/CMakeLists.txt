# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QGeoSatelliteInfoSourceTestPlugin
    OUTPUT_NAME qtposition_satellitesourcetest
    CLASS_NAME QGeoSatelliteInfoSourceTest
    PLUGIN_TYPE position
    DEFAULT_IF FALSE
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
)

