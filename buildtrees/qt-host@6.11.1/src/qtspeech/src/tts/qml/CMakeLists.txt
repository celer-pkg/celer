# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(TextToSpeechQml
    URI "QtTextToSpeech"
    VERSION "${PROJECT_VERSION}"
    PLUGIN_TARGET TextToSpeechQml
    SOURCES
        qtexttospeech_qmltypes_p.h
        qdeclarativetexttospeech.cpp qdeclarativetexttospeech_p.h
        qvoiceselectorattached.cpp qvoiceselectorattached_p.h
    LIBRARIES
        Qt::TextToSpeech
    NO_GENERATE_CPP_EXPORTS
)
