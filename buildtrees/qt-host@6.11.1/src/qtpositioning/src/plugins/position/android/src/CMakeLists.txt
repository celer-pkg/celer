# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from src.pro.

#####################################################################
## QGeoPositionInfoSourceFactoryAndroid Plugin:
#####################################################################

qt_internal_add_plugin(QGeoPositionInfoSourceFactoryAndroidPlugin
    OUTPUT_NAME qtposition_android
    CLASS_NAME QGeoPositionInfoSourceFactoryAndroid
    PLUGIN_TYPE position
    SOURCES
        jnipositioning.cpp jnipositioning.h
        positionfactory_android.cpp positionfactory_android.h
        qgeopositioninfosource_android.cpp qgeopositioninfosource_android_p.h
        qgeosatelliteinfosource_android.cpp qgeosatelliteinfosource_android_p.h
    NO_UNITY_BUILD_SOURCES
        qgeosatelliteinfosource_android.cpp # explicit specialization of 'QMetaTypeId<QList<QGeoSatelliteInfo>>' after instantiation
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Positioning
)

#### Keys ignored in scope 1:.:.:src.pro:<TRUE>:
# OTHER_FILES = "plugin.json"
