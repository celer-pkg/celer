# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from positionpoll.pro.

#####################################################################
## QGeoPositionInfoSourceFactoryPoll Plugin:
#####################################################################

qt_internal_add_plugin(QGeoPositionInfoSourceFactoryPollPlugin
    OUTPUT_NAME qtposition_positionpoll
    CLASS_NAME QGeoPositionInfoSourceFactoryPoll
    PLUGIN_TYPE position
    SOURCES
        positionpollfactory.cpp positionpollfactory.h
        qgeoareamonitor_polling.cpp qgeoareamonitor_polling.h
    LIBRARIES
        Qt::Core
        Qt::Positioning
)

#### Keys ignored in scope 1:.:.:positionpoll.pro:<TRUE>:
# OTHER_FILES = "plugin.json"
