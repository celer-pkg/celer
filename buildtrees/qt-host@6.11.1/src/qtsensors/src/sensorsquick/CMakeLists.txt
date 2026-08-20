# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(SensorsQuick
    URI "QtSensors"
    VERSION "${PROJECT_VERSION}"
    SOURCES
        qmlaccelerometer.cpp qmlaccelerometer_p.h
        qmlambientlightsensor.cpp qmlambientlightsensor_p.h
        qmlambienttemperaturesensor.cpp qmlambienttemperaturesensor_p.h
        qmlcompass.cpp qmlcompass_p.h
        qmlgyroscope.cpp qmlgyroscope_p.h
        qmlhumiditysensor.cpp qmlhumiditysensor_p.h
        qmlirproximitysensor.cpp qmlirproximitysensor_p.h
        qmllidsensor.cpp qmllidsensor_p.h
        qmllightsensor.cpp qmllightsensor_p.h
        qmlmagnetometer.cpp qmlmagnetometer_p.h
        qmlorientationsensor.cpp qmlorientationsensor_p.h
        qmlpressuresensor.cpp qmlpressuresensor_p.h
        qmlproximitysensor.cpp qmlproximitysensor_p.h
        qmlrotationsensor.cpp qmlrotationsensor_p.h
        qmlsensor.cpp qmlsensor_p.h
        qmlsensorglobal.cpp qmlsensorglobal_p.h
        qmlsensorrange.cpp qmlsensorrange_p.h
        qmltapsensor.cpp qmltapsensor_p.h
        qmltiltsensor.cpp qmltiltsensor_p.h
        qsensorsquickglobal_p.h
    DEFINES
        QT_BUILD_SENSORSQUICK_LIB
    DEPENDENCIES
        QtQml
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Sensors
    LIBRARIES
        Qt::CorePrivate
)
