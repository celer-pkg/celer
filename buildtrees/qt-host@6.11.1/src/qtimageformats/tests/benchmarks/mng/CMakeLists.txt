# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_bench_qmng Binary:
#####################################################################

qt_internal_add_benchmark(tst_bench_qmng
    SOURCES
        tst_qmng.cpp
    LIBRARIES
        Qt::Gui
        Qt::Test
)

# Resources:
set(mng_resource_files
    "../../shared/images/mng/animation.mng"
    "../../shared/images/mng/ball.mng"
    "../../shared/images/mng/corrupt.mng"
    "../../shared/images/mng/dutch.mng"
    "../../shared/images/mng/fire.mng"
)

qt_internal_add_resource(tst_bench_qmng "mng"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${mng_resource_files}
)
