# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QTextToSpeechMockPlugin
    OUTPUT_NAME qtexttospeech_mock
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_mock.cpp qtexttospeech_mock.h
        qtexttospeech_mock_plugin.cpp qtexttospeech_mock_plugin.h
    LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)
