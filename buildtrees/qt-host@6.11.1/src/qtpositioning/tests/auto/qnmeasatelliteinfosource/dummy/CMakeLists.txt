# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin
qt_internal_add_test(tst_dummynmeasatelliteinfosource
    SOURCES
        ../../utils/qlocationtestutils.cpp ../../utils/qlocationtestutils_p.h
        ../../utils/qnmeaproxyfactory.cpp ../../utils/qnmeaproxyfactory.h
        tst_dummynmeasatelliteinfosource.cpp
    INCLUDE_DIRECTORIES
        ..
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Positioning
)
# special case end
