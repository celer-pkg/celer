# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## dummySensorPlugin Plugin:
#####################################################################

qt_internal_add_plugin(dummySensorPlugin
    OUTPUT_NAME qtsensors_dummy
    PLUGIN_TYPE sensors
    SOURCES
        dummyaccelerometer.cpp dummyaccelerometer.h
        dummycommon.cpp dummycommon.h
        dummylightsensor.cpp dummylightsensor.h
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Sensors
)

#### Keys ignored in scope 1:.:.:dummy.pro:<TRUE>:
# OTHER_FILES = "plugin.json"

## Scopes:
#####################################################################

qt_internal_extend_target(dummySensorPlugin CONDITION UNIX AND NOT ANDROID AND NOT APPLE AND NOT OPENBSD AND NOT QNX
    LIBRARIES
        rt
)
