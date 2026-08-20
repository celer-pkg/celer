# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## genericSensorPlugin Plugin:
#####################################################################

qt_internal_add_plugin(genericSensorPlugin
    OUTPUT_NAME qtsensors_generic
    PLUGIN_TYPE sensors
    SOURCES
        genericalssensor.cpp genericalssensor.h
        genericorientationsensor.cpp genericorientationsensor.h
        generictiltsensor.cpp generictiltsensor.h
        main.cpp
    DEFINES
        QTSENSORS_GENERICALSSENSOR
        QTSENSORS_GENERICORIENTATIONSENSOR
        QTSENSORS_GENERICTILTSENSOR
    LIBRARIES
        Qt::Core
        Qt::Sensors
)

#### Keys ignored in scope 1:.:.:generic.pro:<TRUE>:
# OTHER_FILES = "plugin.json"

## Scopes:
#####################################################################

qt_internal_extend_target(genericSensorPlugin CONDITION NOT ANDROID
    SOURCES
        genericrotationsensor.cpp genericrotationsensor.h
    DEFINES
        QTSENSORS_GENERICROTATIONSENSOR
)
