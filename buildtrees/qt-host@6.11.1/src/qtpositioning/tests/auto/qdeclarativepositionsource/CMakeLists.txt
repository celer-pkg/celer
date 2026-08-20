# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qdeclarativepositionsource
    SOURCES
        tst_qdeclarativepositionsource.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
        Qt::PositioningQuickPrivate
        Qt::TestPrivate
)

add_dependencies(tst_qdeclarativepositionsource QGeoPositionInfoSourceFactoryTestPlugin
    DummyPluginForTestsFactoryPlugin)
if(ANDROID)
    # We depend on 2 plugins, but they both will be installed in the same dir
    set_target_properties(tst_qdeclarativepositionsource PROPERTIES
        QT_ANDROID_EXTRA_PLUGINS "$<TARGET_FILE_DIR:QGeoPositionInfoSourceFactoryTestPlugin>"
    )
endif()
