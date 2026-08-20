# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qgeocoordinate.pro.

#####################################################################
## tst_qgeocoordinate Test:
#####################################################################

qt_internal_add_test(tst_qgeocoordinate
    SOURCES
        ../utils/qlocationtestutils.cpp ../utils/qlocationtestutils_p.h
        tst_qgeocoordinate.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
)
