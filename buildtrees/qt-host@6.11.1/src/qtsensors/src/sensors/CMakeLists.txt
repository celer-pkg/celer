# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Sensors Module:
#####################################################################

qt_internal_add_module(Sensors
    PLUGIN_TYPES sensors
    SOURCES
    qsensorbackend.cpp qsensorbackend.h
    qsensormanager.cpp qsensormanager.h
    qsensorplugin.cpp qsensorplugin.h
    qsensorsglobal.h
    sensorlog_p.h
    qsensor.h
    qaccelerometer.h
    qambientlightsensor.h
    qambienttemperaturesensor.h
    qcompass.h
    qlightsensor.h
    qmagnetometer.h
    qorientationsensor.h
    qproximitysensor.h
    qirproximitysensor.h
    qrotationsensor.h
    qtapsensor.h
    qtiltsensor.h
    qgyroscope.h
    qpressuresensor.h
    qhumiditysensor.h
    qlidsensor.h
    qsensor_p.h
    qaccelerometer_p.h
    qambientlightsensor_p.h
    qambienttemperaturesensor_p.h
    qcompass_p.h
    qlightsensor_p.h
    qmagnetometer_p.h
    qorientationsensor_p.h
    qproximitysensor_p.h
    qirproximitysensor_p.h
    qrotationsensor_p.h
    qtapsensor_p.h
    qtiltsensor_p.h
    qgyroscope_p.h
    qpressuresensor_p.h
    qhumiditysensor_p.h
    qlidsensor_p.h
    qsensor.cpp
    qaccelerometer.cpp
    qambientlightsensor.cpp
    qambienttemperaturesensor.cpp
    qcompass.cpp
    qlightsensor.cpp
    qmagnetometer.cpp
    qorientationsensor.cpp
    qproximitysensor.cpp
    qirproximitysensor.cpp
    qrotationsensor.cpp
    qtapsensor.cpp
    qtiltsensor.cpp
    qgyroscope.cpp
    qpressuresensor.cpp
    qhumiditysensor.cpp
    qlidsensor.cpp
    DEFINES
        $<$<CONFIG:Debug>:ENABLE_RUNTIME_SENSORLOG>
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

if(ANDROID)
    set_property(TARGET Sensors APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        ${INSTALL_PLUGINSDIR}/sensors/libplugins_sensors_qtsensors_android.so
    )
endif()

## Scopes:
#####################################################################

qt_internal_extend_target(Sensors CONDITION SENSORS_CONFIG_PATH
    DEFINES
        QTSENSORS_CONFIG_PATH="${SENSORS_CONFIG_PATH}"
)

qt_internal_add_docs(Sensors
    doc/qtsensors.qdocconf
)

if(QT_FEATURE_doc_snippets)
    add_subdirectory(doc/snippets/sensors)
endif()
