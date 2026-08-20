# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qgeoareamonitor.pro.

#####################################################################
## tst_qgeoareamonitor Test:
#####################################################################

list(APPEND test_data "simplelog.txt")

qt_internal_add_test(tst_qgeoareamonitor
    SOURCES
        logfilepositionsource.cpp logfilepositionsource.h
        positionconsumerthread.cpp positionconsumerthread.h
        tst_qgeoareamonitor.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
    TESTDATA ${test_data}
)

# QGeoAreaMonitor positionpoll plugin requires the presence of a
# QGeoPositionInfoSource object. It could be provided by android position
# plugin, but that plugin is not loaded for unit-tests, so we have to explicitly
# use a test position plugin.
add_dependencies(tst_qgeoareamonitor QGeoPositionInfoSourceFactoryTestPlugin)
if(ANDROID)
    set_target_properties(tst_qgeoareamonitor PROPERTIES
        QT_ANDROID_EXTRA_PLUGINS "$<TARGET_FILE_DIR:QGeoPositionInfoSourceFactoryTestPlugin>"
    )
endif()

#### Keys ignored in scope 1:.:.:qgeoareamonitor.pro:<TRUE>:
# OTHER_FILES = "*.txt"
# TEMPLATE = "app"
