# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QCounterGesturePlugin Plugin:
#####################################################################

qt_internal_add_plugin(AndroidSensorPlugin
    OUTPUT_NAME qtsensors_android
    PLUGIN_TYPE sensors
    SOURCES
        androidaccelerometer.cpp androidaccelerometer.h
        androidcompass.cpp androidcompass.h
        androidgyroscope.cpp androidgyroscope.h
        androidlight.cpp androidlight.h
        androidmagnetometer.cpp androidmagnetometer.h
        androidpressure.cpp androidpressure.h
        androidproximity.cpp androidproximity.h
        androidrotation.cpp androidrotation.h
        androidtemperature.cpp androidtemperature.h
        main.cpp
        sensoreventqueue.h
        sensormanager.cpp sensormanager.h
    DEFINES
        # STATICPLUGIN needed because there's a Q_IMPORT_PLUGIN in main.cpp
        # Yes, the plugin imports itself statically
        QT_STATICPLUGIN
    LIBRARIES
        Qt::CorePrivate
        Qt::Sensors
        android
)

#### Keys ignored in scope 1:.:.:android.pro:<TRUE>:
# OTHER_FILES = "plugin.json"
