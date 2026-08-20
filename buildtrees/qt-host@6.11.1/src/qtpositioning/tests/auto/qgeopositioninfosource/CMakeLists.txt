# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qgeopositioninfosource.pro.

#####################################################################
## tst_qgeopositioninfosource Test:
#####################################################################

qt_internal_add_test(tst_qgeopositioninfosource
    SOURCES
        ../utils/qlocationtestutils.cpp ../utils/qlocationtestutils_p.h
        testqgeopositioninfosource.cpp testqgeopositioninfosource_p.h
        tst_qgeopositioninfosource.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
        Qt::TestPrivate
)

add_dependencies(tst_qgeopositioninfosource QGeoPositionInfoSourceFactoryTestPlugin)
if(ANDROID)
    set_target_properties(tst_qgeopositioninfosource PROPERTIES
        QT_ANDROID_EXTRA_PLUGINS "$<TARGET_FILE_DIR:QGeoPositionInfoSourceFactoryTestPlugin>"
    )
endif()

#### Keys ignored in scope 1:.:.:qgeopositioninfosource.pro:<TRUE>:
# TEMPLATE = "app"
# testcase.timeout = "400"
