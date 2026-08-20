# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from corelocation.pro.

#####################################################################
## QGeoPositionInfoSourceFactoryCL Plugin:
#####################################################################

qt_internal_add_plugin(QGeoPositionInfoSourceFactoryCLPlugin
    OUTPUT_NAME qtposition_cl
    CLASS_NAME QGeoPositionInfoSourceFactoryCL
    PLUGIN_TYPE position
    SOURCES
        qgeopositioninfosource_cl.mm qgeopositioninfosource_cl_p.h
        qgeopositioninfosourcefactory_cl.h qgeopositioninfosourcefactory_cl.mm
    LIBRARIES
        ${FWCoreLocation}
        ${FWFoundation}
        Qt::Core
        Qt::CorePrivate
        Qt::Positioning
)

# special case begin
set_target_properties(QGeoPositionInfoSourceFactoryCLPlugin
    PROPERTIES
        DISABLE_PRECOMPILE_HEADERS ON
)
# special case end

#### Keys ignored in scope 1:.:.:corelocation.pro:<TRUE>:
# OTHER_FILES = "plugin.json"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:corelocation.pro:NOT APPLE:
# DISTFILES = "$$OBJECTIVE_SOURCES"
