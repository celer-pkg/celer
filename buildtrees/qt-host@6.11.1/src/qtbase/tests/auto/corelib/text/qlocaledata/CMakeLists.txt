# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qlocaledata Test:
#####################################################################

qt_internal_add_test(tst_qlocaledata
    SOURCES
        tst_qlocaledata.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::TestPrivate
)
