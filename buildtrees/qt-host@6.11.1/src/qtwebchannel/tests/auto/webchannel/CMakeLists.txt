# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_webchannel
    SOURCES
        tst_webchannel.cpp tst_webchannel.h
    LIBRARIES
        Qt::CorePrivate
        Qt::WebChannelPrivate
        Qt::TestPrivate
)

qt_internal_extend_target(tst_webchannel CONDITION TARGET Qt::Qml
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_JS_ENGINE
    LIBRARIES
        Qt::Qml
)

qt_internal_extend_target(tst_webchannel CONDITION TARGET Qt::Concurrent
    DEFINES
        WEBCHANNEL_TESTS_CAN_USE_CONCURRENT
    LIBRARIES
        Qt::Concurrent
)
