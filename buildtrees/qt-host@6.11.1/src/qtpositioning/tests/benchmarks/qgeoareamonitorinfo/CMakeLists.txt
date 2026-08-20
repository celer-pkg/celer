# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin

qt_internal_add_benchmark(tst_bench_qgeoareamonitorinfo
    SOURCES
        tst_bench_qgeoareamonitorinfo.cpp
    LIBRARIES
        Qt::Core
        Qt::Positioning
        Qt::Test
)

# special case end
