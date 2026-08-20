# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from geoclue2.pro.

#####################################################################
## QGeoPositionInfoSourceFactoryGeoclue2 Plugin:
#####################################################################

qt_internal_add_plugin(QGeoPositionInfoSourceFactoryGeoclue2Plugin
    OUTPUT_NAME qtposition_geoclue2
    CLASS_NAME QGeoPositionInfoSourceFactoryGeoclue2
    PLUGIN_TYPE position
    SOURCES
        geocluetypes.cpp geocluetypes.h
        qgeopositioninfosource_geoclue2.cpp qgeopositioninfosource_geoclue2_p.h
        qgeopositioninfosourcefactory_geoclue2.cpp qgeopositioninfosourcefactory_geoclue2.h
    DBUS_INTERFACE_SOURCES
        org.freedesktop.GeoClue2.Client.xml
        org.freedesktop.GeoClue2.Location.xml
        org.freedesktop.GeoClue2.Manager.xml
    DBUS_INTERFACE_FLAGS
        -N -i geocluetypes.h # special case
        --namespace QtPositioningPrivate
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_BINARY_DIR}
        core.location.includes
    LIBRARIES
        Qt::Core
        Qt::DBus
        Qt::Positioning
)

#### Keys ignored in scope 1:.:.:geoclue2.pro:<TRUE>:
# OTHER_FILES = "plugin.json"
