# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from dummynmeapositioninfosource.pro.

#####################################################################
## tst_dummynmeapositioninfosource Test:
#####################################################################

qt_internal_add_test(tst_dummynmeapositioninfosource
    SOURCES
        ../../utils/qlocationtestutils.cpp ../../utils/qlocationtestutils_p.h
        ../../utils/qnmeaproxyfactory.cpp ../../utils/qnmeaproxyfactory.h
        tst_dummynmeapositioninfosource.cpp
    INCLUDE_DIRECTORIES
        ..
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Positioning
)

#### Keys ignored in scope 1:.:.:dummynmeapositioninfosource.pro:<TRUE>:
# TEMPLATE = "app"
