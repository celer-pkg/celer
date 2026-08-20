# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(TextToSpeech
    PLUGIN_TYPES texttospeech
    SOURCES
        qtexttospeech.cpp qtexttospeech.h qtexttospeech_p.h
        qtexttospeech_global.h
        qtexttospeechengine.cpp qtexttospeechengine.h
        qtexttospeechplugin.cpp qtexttospeechplugin.h
        qvoice.cpp qvoice.h qvoice_p.h
    DEFINES
        QTEXTTOSPEECH_LIBRARY
        QT_NO_CONTEXTLESS_CONNECT
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Multimedia
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
    NO_GENERATE_CPP_EXPORTS
)


if(TARGET Qt::Qml)
    add_subdirectory(qml)
endif()

if(ANDROID)
    set_property(TARGET TextToSpeech APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtSpeech_VERSION_MAJOR}AndroidTextToSpeech.jar
    )
    set_property(TARGET TextToSpeech APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        ${INSTALL_PLUGINSDIR}/texttospeech/libplugins_texttospeech_qttexttospeech_android.so
    )
endif()

qt_internal_add_docs(TextToSpeech
    doc/qttexttospeech.qdocconf
)
