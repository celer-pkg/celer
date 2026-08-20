# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## WinRtSensorPlugin Plugin:
#####################################################################

qt_internal_add_plugin(WinRtSensorPlugin
    OUTPUT_NAME qtsensors_winrt
    PLUGIN_TYPE sensors
    SOURCES
        main.cpp
        winrtaccelerometer.cpp winrtaccelerometer.h
        winrtambientlightsensor.cpp winrtambientlightsensor.h
        winrtcommon.cpp winrtcommon.h
        winrtcompass.cpp winrtcompass.h
        winrtgyroscope.cpp winrtgyroscope.h
        winrtorientationsensor.cpp winrtorientationsensor.h
        winrtrotationsensor.cpp winrtrotationsensor.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Sensors
)

qt_internal_extend_target(WinRtSensorPlugin CONDITION MSVC
    LIBRARIES
        runtimeobject
)

#### Keys ignored in scope 1:.:.:winrt.pro:<TRUE>:
# OTHER_FILES = "plugin.json"
