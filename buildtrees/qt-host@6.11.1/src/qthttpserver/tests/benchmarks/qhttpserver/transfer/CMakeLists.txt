# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_benchmark(tst_bench_qhttpserver_transfer
    SOURCES
        tst_bench_qhttpserver_transfer.cpp
    LIBRARIES
        Qt::HttpServer
        Qt::Test
)

qt_add_resources(tst_bench_qhttpserver_transfer "certs"
    PREFIX "/cert"
    BASE "data/"
    FILES
        "data/localhost.cert"
        "data/localhost.key"
)
