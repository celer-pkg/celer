# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## mediaplayer Binary:
#####################################################################

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt_internal_add_manual_test(tst_mediaplayer
    SOURCES
        mainwindow.ui
        mediaaxwidget.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::AxContainer
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

set_target_properties(tst_mediaplayer PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
