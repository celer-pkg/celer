# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_find_apple_system_framework(FWAVFoundation AVFoundation)

qt_internal_add_plugin(QTextToSpeechDarwinPlugin
    OUTPUT_NAME qtexttospeech_speechdarwin
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_darwin.h qtexttospeech_darwin.mm
        qtexttospeech_darwin_plugin.cpp qtexttospeech_darwin_plugin.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
        Qt::Multimedia
        ${FWAVFoundation}
)
