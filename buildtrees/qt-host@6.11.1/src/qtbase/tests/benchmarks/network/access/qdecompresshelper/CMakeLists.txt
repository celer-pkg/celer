# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_bench_qdecompresshelper Binary:
#####################################################################

qt_internal_add_benchmark(tst_bench_qdecompresshelper
    SOURCES
        main.cpp
    DEFINES
        SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        Qt::NetworkPrivate
        Qt::Test
)
