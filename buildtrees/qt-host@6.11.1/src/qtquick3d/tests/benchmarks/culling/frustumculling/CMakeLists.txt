# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(benchmark_frustumculling
    SOURCES
        tst_benchfrustumculling.cpp
    LIBRARIES
        Qt::Test
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRenderPrivate
)

