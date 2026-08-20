# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qvoice
    SOURCES
        tst_qvoice.cpp
    LIBRARIES
        Qt::TextToSpeechPrivate
)
