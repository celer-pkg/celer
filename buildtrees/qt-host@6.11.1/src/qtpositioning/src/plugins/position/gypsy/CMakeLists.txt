# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from gypsy.pro.

#####################################################################
## QGeoPositionInfoSourceFactoryGypsy Plugin:
#####################################################################

qt_internal_add_plugin(QGeoPositionInfoSourceFactoryGypsyPlugin
    OUTPUT_NAME qtposition_gypsy
    CLASS_NAME QGeoPositionInfoSourceFactoryGypsy
    PLUGIN_TYPE position
    SOURCES
        qgeopositioninfosourcefactory_gypsy.cpp qgeopositioninfosourcefactory_gypsy.h
        qgeosatelliteinfosource_gypsy.cpp qgeosatelliteinfosource_gypsy_p.h
# special case begin
    LIBRARIES
        Gypsy::Gypsy
        Gconf::Gconf
        Qt::Core
        Qt::Positioning
# special case end
)

#### Keys ignored in scope 1:.:.:gypsy.pro:<TRUE>:
# OTHER_FILES = "plugin.json"
