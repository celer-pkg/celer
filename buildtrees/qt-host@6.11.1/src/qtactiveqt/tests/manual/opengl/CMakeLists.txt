# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## testcontrol Binary:
#####################################################################

set(CMAKE_AUTOMOC ON)

qt_internal_add_manual_test(tst_opengl
    GUI
    SOURCES
        glbox.cpp glbox.h
        globjwin.cpp globjwin.h
        main.cpp
        opengl.def
        opengl.rc
    LIBRARIES
        Qt::AxServer
        Qt::Gui
        Qt::OpenGL
        Qt::OpenGLWidgets
        Qt::Widgets
)

# qt_disable_warnings(tst_opengl)
qt6_target_idl(tst_opengl)
