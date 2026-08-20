# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QTextToSpeechEngineAndroidPlugin
    OUTPUT_NAME qttexttospeech_android
    PLUGIN_TYPE texttospeech
    CLASS_NAME QTextToSpeechEngineAndroid
    SOURCES
        qtexttospeech_android.cpp qtexttospeech_android.h
        qtexttospeech_android_plugin.cpp qtexttospeech_android_plugin.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::TextToSpeech
        Qt::Multimedia
)

add_dependencies(QTextToSpeechEngineAndroidPlugin Qt${QtSpeech_VERSION_MAJOR}AndroidTextToSpeech)
