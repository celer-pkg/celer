# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## sensorfwSensorPlugin Plugin:
#####################################################################

qt_internal_add_plugin(sensorfwSensorPlugin
    OUTPUT_NAME qtsensors_sensorfw
    PLUGIN_TYPE sensors
    SOURCES
        main.cpp
        sensorfwaccelerometer.cpp sensorfwaccelerometer.h
        sensorfwals.cpp sensorfwals.h
        sensorfwcompass.cpp sensorfwcompass.h
        sensorfwgyroscope.cpp sensorfwgyroscope.h
        sensorfwirproximitysensor.cpp sensorfwirproximitysensor.h
        sensorfwlidsensor.cpp sensorfwlidsensor.h
        sensorfwlightsensor.cpp sensorfwlightsensor.h
        sensorfwmagnetometer.cpp sensorfwmagnetometer.h
        sensorfworientationsensor.cpp sensorfworientationsensor.h
        sensorfwproximitysensor.cpp sensorfwproximitysensor.h
        sensorfwrotationsensor.cpp sensorfwrotationsensor.h
        sensorfwsensorbase.cpp sensorfwsensorbase.h
        sensorfwtapsensor.cpp sensorfwtapsensor.h
    LIBRARIES
        Qt::Core
        Qt::DBus
        Qt::Network
        Qt::SensorsPrivate
        sensorfw
)

#### Keys ignored in scope 1:.:.:sensorfw.pro:<TRUE>:
# CONFIGFILES.files = "Sensors.conf"
# CONFIGFILES.path = "/etc/xdg/QtProject/"
# INSTALLS = "CONFIGFILES"
# OTHER_FILES = "plugin.json"
