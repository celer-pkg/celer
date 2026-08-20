# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_bench_qsggeometry Binary:
#####################################################################

qt_internal_add_benchmark(tst_bench_qsggeometry
    SOURCES
        tst_bench_qsggeometry.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Test
)
