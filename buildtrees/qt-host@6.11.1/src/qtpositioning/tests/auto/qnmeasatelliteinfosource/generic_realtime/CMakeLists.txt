# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin
qt_internal_add_test(tst_nmeasatelliteinfosource_generic_rt
    SOURCES
        ../../qgeosatelliteinfosource/testqgeosatelliteinfosource.cpp ../../qgeosatelliteinfosource/testqgeosatelliteinfosource_p.h
        ../../utils/qlocationtestutils.cpp ../../utils/qlocationtestutils_p.h
        ../../utils/qnmeaproxyfactory.cpp ../../utils/qnmeaproxyfactory.h
        tst_nmeasatelliteinfosource_generic_rt.cpp
    INCLUDE_DIRECTORIES
        ..
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Positioning
        Qt::TestPrivate
)
# special case end
