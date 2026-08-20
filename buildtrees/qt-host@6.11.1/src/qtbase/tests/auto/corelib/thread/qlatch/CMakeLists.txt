# Copyright (C) 2025 Intel Corporation.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qlatch
    SOURCES
        tst_qlatch.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Test
)

qt_internal_add_test(tst_qlatch_fallback
    SOURCES
        tst_qlatch.cpp
        ../../../../../src/corelib/thread/qatomicwait.cpp
        ../../../../../src/corelib/thread/qlatch.cpp
    DEFINES
        QATOMICWAIT_USE_FALLBACK
    LIBRARIES
        Qt::CorePrivate
        Qt::Test
)

qt_internal_extend_target(tst_qlatch_fallback CONDITION WIN32
    LIBRARIES
        synchronization
)
