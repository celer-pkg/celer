# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin
qt_internal_add_test(tst_nmeasatelliteinfosource
    SOURCES
        ../../utils/qlocationtestutils.cpp ../../utils/qlocationtestutils_p.h
        tst_nmeasatelliteinfosource.cpp
    INCLUDE_DIRECTORIES
        ..
    LIBRARIES
        Qt::Core
        Qt::Positioning
)
# special case end
