# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## IOSSensorPlugin Plugin:
#####################################################################

qt_internal_add_plugin(IOSSensorPlugin
    OUTPUT_NAME qtsensors_ios
    PLUGIN_TYPE sensors
    SOURCES
        main.mm
    LIBRARIES
        ${FWFoundation}
        Qt::Core
        Qt::Sensors
)

#### Keys ignored in scope 1:.:.:ios.pro:<TRUE>:
# OTHER_FILES = "plugin.json"

## Scopes:
#####################################################################

set_target_properties(IOSSensorPlugin
    PROPERTIES
        DISABLE_PRECOMPILE_HEADERS ON
)

qt_internal_extend_target(IOSSensorPlugin CONDITION IOS AND UIKIT
    SOURCES
        ioscompass.h ioscompass.mm
    DEFINES
        HAVE_COMPASS
    LIBRARIES
        ${FWCoreLocation}
)

qt_internal_extend_target(IOSSensorPlugin CONDITION UIKIT AND NOT TVOS
    SOURCES
        iosaccelerometer.h iosaccelerometer.mm
        iosgyroscope.h iosgyroscope.mm
        iosmagnetometer.h iosmagnetometer.mm
        iospressure.h iospressure.mm
        iosmotionmanager.h iosmotionmanager.mm
    DEFINES
        HAVE_COREMOTION
    LIBRARIES
        ${FWCoreMotion}
)

qt_internal_extend_target(IOSSensorPlugin CONDITION UIKIT AND NOT WATCHOS
    SOURCES
        iosproximitysensor.h iosproximitysensor.mm
    DEFINES
        HAVE_UIDEVICE
    LIBRARIES
        ${FWUIKit}
)
