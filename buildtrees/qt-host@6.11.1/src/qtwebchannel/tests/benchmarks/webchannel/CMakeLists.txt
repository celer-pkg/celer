# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_benchmark(tst_bench_qwebchannel
    SOURCES
        tst_bench_qwebchannel.cpp tst_bench_qwebchannel.h
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannel
        Qt::WebChannelPrivate
        Qt::Test
)
