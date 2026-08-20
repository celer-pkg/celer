# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qtexttospeech
    SOURCES
        tst_qtexttospeech.cpp
    LIBRARIES
        Qt::TextToSpeechPrivate
        Qt::Multimedia
)

# on macOS we need to run a Cocoa event dispatcher
qt_internal_extend_target(tst_qtexttospeech CONDITION MACOS
    LIBRARIES
        Qt::Gui
)

# for a standalone test build we have to find_package SpeechDispatcher again
find_package(SpeechDispatcher)
qt_internal_extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
