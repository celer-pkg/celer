# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qnmeapositioninfosource_realtime.pro.

#####################################################################
## tst_qnmeapositioninfosource_realtime Test:
#####################################################################

qt_internal_add_test(tst_qnmeapositioninfosource_realtime
    SOURCES
        ../../qgeopositioninfosource/testqgeopositioninfosource.cpp ../../qgeopositioninfosource/testqgeopositioninfosource_p.h
        ../../utils/qlocationtestutils.cpp ../../utils/qlocationtestutils_p.h
        ../../utils/qnmeaproxyfactory.cpp ../../utils/qnmeaproxyfactory.h
        ../tst_qnmeapositioninfosource.cpp ../tst_qnmeapositioninfosource.h
        tst_qnmeapositioninfosource_realtime.cpp
    INCLUDE_DIRECTORIES
        ..
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Positioning
        Qt::TestPrivate
)

#### Keys ignored in scope 1:.:.:qnmeapositioninfosource_realtime.pro:<TRUE>:
# TEMPLATE = "app"
