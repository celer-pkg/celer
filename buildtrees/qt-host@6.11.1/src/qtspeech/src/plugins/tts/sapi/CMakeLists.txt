# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QTextToSpeechSapiPlugin
    OUTPUT_NAME qtexttospeech_sapi
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_sapi.cpp qtexttospeech_sapi.h
        qtexttospeech_sapi_plugin.cpp qtexttospeech_sapi_plugin.h
    LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)

qt_internal_extend_target(QTextToSpeechSapiPlugin CONDITION MSVC
    COMPILE_OPTIONS
        /Zc:strictStrings
)

qt_internal_extend_target(QTextToSpeechSapiPlugin CONDITION MINGW
    LIBRARIES
        ole32
        sapi
        uuid
)
