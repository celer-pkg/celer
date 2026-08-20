# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qcamerabackend Test:
#####################################################################

qt_internal_add_test(tst_qmediacapturesession
    SOURCES
        tst_qmediacapturesession.cpp
    LIBRARIES
        Qt::Gui
        Qt::MultimediaPrivate
        Qt::MultimediaWidgets
        Qt::MultimediaTestLibPrivate
)

if(APPLE)
    set_target_properties(tst_qmediacapturesession PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
endif()
