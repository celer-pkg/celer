# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries

qt_find_package(Flite PROVIDED_TARGETS Flite::Flite MODULE_NAME texttospeech QMAKE_LIB flite)
qt_find_package(SpeechDispatcher PROVIDED_TARGETS SpeechDispatcher::SpeechDispatcher MODULE_NAME texttospeech QMAKE_LIB speechd)


#### Tests



#### Features

qt_feature("flite" PRIVATE
    LABEL "Flite"
    CONDITION Flite_FOUND
)
qt_feature("speechd" PUBLIC
    LABEL "Speech Dispatcher"
    AUTODETECT UNIX
    CONDITION SpeechDispatcher_FOUND
)
qt_configure_add_summary_section(NAME "Qt TextToSpeech")
qt_configure_add_summary_entry(ARGS "flite")
qt_configure_add_summary_entry(ARGS "speechd")
qt_configure_end_summary_section() # end of "Qt TextToSpeech" section
